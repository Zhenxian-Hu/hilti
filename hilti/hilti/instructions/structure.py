# $Id$
"""
.. hlt:type:: struct

   The ``struct`` data type groups a set of heterogenous, named fields. Each
   instance tracks which fields have already been set. Fields may optionally
   provide defaul values that are returned when it's read but hasn't been set
   yet.
"""

builtin_id = id

import llvm.core

import hilti.util as util

from hilti.constraints import *
from hilti.instructions.operators import *

@hlt.type("struct", 7)
class Struct(type.HeapType, type.TypeListable):
    """Type for ``struct``.

    fields: list of (~~ID, ~~Operand) - The fields of the struct, given as
    tuples of an ID and an optional default value; if a field does not have a
    default value, use None as the operand. If an ID is of ~~INTERNAL linkage,
    it will not be printed when the struct type in rendered as a string.
    Internal IDS are also skipped from for ctor expressions and list
    conversions.

    location: ~~Location - Location information for the type.
    """
    def __init__(self, fields, location=None):
        super(Struct, self).__init__(location=location)
        self._ids = fields

        # Maps field name to (idx, id).
        self._idmap = dict([(t[0].name(), (n, t[0])) for (n, t) in zip(range(len(fields)), fields)])

    def fields(self):
        """Returns the struct's fields.

        Returns: list of (~~ID, ~~Operand) - The struct's fields, given as
        tuples of an ID and an optional default value.
        """
        return self._ids

    def field(self, name):
        """Returns teh struct's field of the given name.

        Returns: ~~ID - The ID for the field, or None if the struct doesn't
        have a field of that name.
        """
        try:
            return self._idmap[name][1]
        except KeyError:
            return None

    def fieldIndex(self, name):
        """Returns teh struct's field index for the given name.

        Returns: int - The index for the field, or -1 if the struct doesn't
        have a field of that name.
        """
        try:
            return self._idmap[name][0]
        except KeyError:
            return -1

    class NotSet:
        """Sentinel to mark an unset fields when listing struct
        values."""
        pass

    def llvmFromValues(self, cg, vals):
        """Creates a new struct instance from a list of values. The individual
        values must already be coerced to the corresponding field's type (if
        necessary). Note that values for all struct fields must be given.
        However, a value can be an instance of ``Struct.NotSet`` to use either
        the default value (if one is defined) or leave the field
        uninitialized.

        vals: list of llvm.core.Value or Struct.NotSet - The values to
        initialize the struct with, excluding any internal fields.

        Returns: llvm.core.Value - The new structure.
        """
        # Fill in defaults and determine masks.
        new_vals = []
        mask = 0
        m = 0
        for n in range(len(self._ids)):

            val = None
            use_default = True

            (i, default) = self._ids[n]

            if i.linkage() == id.Linkage.INTERNAL:
                use_default = True

            else:
                if isinstance(vals[m], Struct.NotSet):
                    use_default = True

                else:
                    val = vals[m]
                    # Mark as set.
                    mask |= (1 << n)

                m += 1

            if use_default and default and not val:
                mask |= (1 << n) # Mark as set.
                val = cg.llvmOp(default, i.type())

            new_vals += [val]

        # Add the mask in front.
        vals = [cg.llvmConstInt(mask, 32)] + new_vals

        stype = self.llvmType(cg).pointee
        init = cg.llvmAlloca(stype)
        init = cg.builder().load(init)

        for (i, val) in zip(range(len(vals)), vals):
            if val:
                init = cg.llvmInsertValue(init, i, val)

        s = cg.llvmMalloc(self.llvmType(cg).pointee, tag="new <struct>")
        cg.llvmAssign(init, s)

        return s

    def llvmValueList(self, cg, s, default=None, func=None):
        """Returns a list with a structure's values.

        cg: ~~CodeGen - The code generator to use.

        s: llvm.core.Value - The structure to extract value's from.

        default: llvm.core.Value - Optional substitute to use if a field is
        not set; if not given, unset fields will trigger an exception.

        func: function - An optional Python function that will be called for
        each field value; its return value will then be inserted into the
        result list instead of the original value. The function will receive
        two arguments: the code generator and the extracted value.

        Returns: list of llvm.core.Value: The values.
        """
        vals = []
        for i in range(len(self._ids)):
            if i[0].linkage() != id.Linkage.INTERNAL:
                vals += self.llvmGetField(cg, s, i, default, func)

        return vals

    def llvmGetField(self, cg, s, idx, default=None, func=None, location=None):
        """Extracts a field from structure.

        cg: ~~CodeGen - The code generator to use.

        s: llvm.core.Value - The structure to extract the value from.

        idx: integer or string - If an integer, the index of the desired
        field, with zero being the first. If a string, the name of the field.

        default: llvm.core.Value - Optional substitute to use if a field is
        not set; if not given, unset fields will trigger an exception.

        func: function - An optional Python function that will be called for
        each field value; its return value will then be inserted into the
        result list instead of the original value. The function will receive
        two arguments: the code generator and the extracted value.

        Returns: llvm.core.Value - The value.
        """
        # Check whether field is valid.
        zero = cg.llvmGEPIdx(0)

        addr = cg.builder().gep(s, [zero, zero])
        mask = cg.builder().load(addr)

        if isinstance(idx, str):
            idx = self._idmap[idx][0]

        bit = cg.llvmConstInt(1<<idx, 32)
        isset = cg.builder().and_(bit, mask)

        block_ok = cg.llvmNewBlock("ok")
        block_not_set = cg.llvmNewBlock("not_set")
        block_done = cg.llvmNewBlock("done")

        notzero = cg.builder().icmp(llvm.core.IPRED_NE, isset, cg.llvmConstInt(0, 32))
        cg.builder().cbranch(notzero, block_ok, block_not_set)

        cg.pushBuilder(block_ok)

        # Load field.
        index = cg.llvmGEPIdx(idx + 1)
        addr = cg.builder().gep(s, [zero, index])
        result_ok = cg.builder().load(addr)

        if func:
            result_ok = func(cg, result_ok)

        block_ok = cg.builder().block

        cg.builder().branch(block_done)
        cg.popBuilder()

        cg.pushBuilder(block_not_set)

        if not default:
            cg.llvmRaiseExceptionByName("hlt_exception_undefined_value", location)

        cg.builder().branch(block_done)
        cg.popBuilder()

        cg.pushBuilder(block_done)

        if default:
            phi = cg.builder().phi(result_ok.type)
            phi.add_incoming(result_ok, block_ok)
            phi.add_incoming(default, block_not_set)
            return phi
        else:
            return result_ok

    ### Overridden from Type.

    _done = None

    def name(self):
        # We need to avoid infinite recursions here. This kind of a hack
        # though ...
        first = False
        if not Struct._done:
            first = True
            Struct._done = {}

        idx = builtin_id(self)

        try:
            return Struct._done[idx]
        except KeyError:
            pass

        Struct._done[idx] = "<recursive struct>"
        name = "struct { %s }" % ", ".join(["%s %s" % (id.type(), id.name()) for (id, op) in self._ids])
        Struct._done[idx] = name

        if first:
            Struct._done = None

        return name

    def _resolve(self, resolver):
        for (i, op) in self._ids:
            i.resolve(resolver)
            if op:
                op.resolve(resolver)

        return self

    def _validate(self, vld):
        for (i, op) in self._ids:
            i.validate(vld);
            if op:
                op.validate(vld);

    def output(self, printer):
        printer.output("struct {", nl=True)
        printer.push()
        first = True
        for (i, op) in self._ids:
            if i.linkage() == id.Linkage.INTERNAL:
                continue

            if not first:
                printer.output(",", nl=True)
            i.output(printer)
            if op:
                printer.output(" &default=")
                op.output(printer)
            first = False

        printer.output("", nl=True)
        printer.pop()
        printer.output("}")

    ### Overridden from HiltiType.

    def typeInfo(self, cg):
        """Type information for a ``struct`` includes the fields' offsets in
        the ``aux`` entry as a concatenation of pairs (ASCIIZ*, offset), where
        ASCIIZ is a field's name, and offset its offset in the value.
        """
        typeinfo = cg.TypeInfo(self)
        typeinfo.to_string = "hlt::struct_to_string";
        typeinfo.hash = "hlt::struct_hash"
        typeinfo.equal = "hlt::struct_equal"
        typeinfo.args = [id.type() for (id, op) in self._ids]
        typeinfo.c_prototype = "void *"

        zero = cg.llvmGEPIdx(0)
        null = llvm.core.Constant.null(self.llvmType(cg))

        array = []
        for i in range(len(self._ids)):

            # Make the field name.
            str = cg.llvmNewGlobalStringConst(cg.nameNewGlobal("struct-field"), self._ids[i][0].name())

            # Calculate the offset.

                # We skip the bitmask here.
            idx = cg.llvmGEPIdx(i + 1)

                # This is a pretty awful hack but I can't find a nicer way to
                # calculate the offsets as *constants*, and this hack is actually also
                # used by LLVM internaly to do sizeof() for constants so it can't be
                # totally disgusting. :-)
            offset = null.gep([zero, idx]).ptrtoint(llvm.core.Type.int(16))
            struct = llvm.core.Constant.struct([str, offset])

            array += [llvm.core.Constant.struct([str, offset])]

        if array:

            name = cg.nameTypeInfo(self) + "-fields"

            const = llvm.core.Constant.array(array[0].type, array)
            glob = cg.llvmNewGlobalConst(name, const)
            glob.linkage = llvm.core.LINKAGE_LINKONCE_ANY

            typeinfo.aux = glob

        else:
            typeinfo.aux = None

        return typeinfo

    def llvmType(self, cg):
        """A ``struct` is passed as a pointer to an eqivalent C struct; the
        fields' types are converted recursively to C using standard rules."""

        if len(self._ids) == 0:
            return cg.llvmTypeGenericPointer()

        th = llvm.core.TypeHandle.new(llvm.core.Type.opaque())

        assert len(self._ids) <= 32
        fields = [llvm.core.Type.int(32)] + [cg.llvmType(id.type()) for (id, default) in self._ids]
        ty = llvm.core.Type.pointer(llvm.core.Type.struct(fields))
        th.type.refine(ty)
        return ty

    ### Overridden from TypeListable.

    def typeList(self):
        return [i[0].type() for i in self._ids if i[0].linkage() != id.Linkage.INTERNAL]

@hlt.constraint("string")
def _fieldName(ty, op, i):
    if not op or not isinstance(op, operand.Operand):
        return (False, "index must be an operand")

    c = op.constant()

    if not c:
        return (False, "index must be constant")

    if not isinstance(ty, type.String):
        return (False, "index must be string")

    for (id, default) in i.op1().type().refType()._ids:
        if c.value() == id.name():
            return (True, "")

    return (False, "%s is not a field name in %s" % (op.value(), i.op1().type().refType()))

@hlt.constraint("any")
def _fieldType(ty, op, i):

    c = i.op2().constant()

    for (id, default) in i.op1().type().refType()._ids:
        if c.value() == id.name():
            if op.canCoerceTo(id.type()):
                return (True, "")
            else:
                return (False, "type must be %s, but is %s" % (id.type(), op.type()))

    assert False

@hlt.overload(New, op1=cType(cStruct), target=cReferenceOfOp(1))
class New(Operator):
    """Allocates a new instance of the structure given as *op1*. All fields
    will initially be unset.
    """
    def _codegen(self, cg):
        # Allocate memory for struct.
        if isinstance(self.op1(), operand.Type):
            structt = self.op1().value()
        else:
            structt = self.op1().value().type()

        if not len(structt._ids):
            return llvm.core.Constant.null(cg.llvmTypeGenericPointer())

        s = cg.llvmMalloc(structt.llvmType(cg).pointee, tag="new <struct>", location=self.location())

        # Initialize fields
        zero = cg.llvmGEPIdx(0)
        mask = 0

        fields = structt._ids
        for j in range(len(fields)):
            (id, default) = fields[j]
            if default:
                # Initialize with default.
                mask |= (1 << j)
                index = cg.llvmGEPIdx(j + 1)
                addr = cg.builder().gep(s, [zero, index])
                cg.llvmAssign(cg.llvmOp(default, id.type()), addr)
            else:
                # Leave untouched. As we keep the bitmask of which fields are
                # set,  we will never access it.
                pass

        # Set mask.
        addr = cg.builder().gep(s, [zero, zero])
        cg.llvmAssign(cg.llvmConstInt(mask, 32), addr)

        cg.llvmStoreInTarget(self, s)

@hlt.instruction("struct.get", op1=cReferenceOf(cStruct), op2=_fieldName, target=_fieldType)
class Get(Instruction):
    """Returns the field named *op2* in the struct referenced by *op1*. The
    field name must be a constant, and the type of the target must match the
    field's type. If a field is requested that has not been set, its default
    value is returned if it has any defined. If it has not, an
    ``UndefinedValue`` exception is raised.
    """
    def _codegen(self, cg):
        (idx, ftype) = _getIndex(self)
        assert idx >= 0

        s = cg.llvmOp(self.op1())
        val = self.op1().type().refType().llvmGetField(cg, s, idx, location=self.location())
        cg.llvmStoreInTarget(self, val)

@hlt.instruction("struct.get_default", op1=cReferenceOf(cStruct), op2=_fieldName, op3=_fieldType, target=_fieldType)
class Get(Instruction):
    """Returns the field named *op2* in the struct referenced by *op1*, or a
    default value *op3* if not set (if the field has a default itself, that
    however has priority). The field name must be a constant, and the type of
    the target must match the field's type, as must that of the default value.
    """
    def _codegen(self, cg):
        (idx, ftype) = _getIndex(self)
        assert idx >= 0

        s = cg.llvmOp(self.op1())
        val = self.op1().type().refType().llvmGetField(cg, s, idx, default=cg.llvmOp(self.op3()), location=self.location())
        cg.llvmStoreInTarget(self, val)

@hlt.instruction("struct.set", op1=cReferenceOf(cStruct), op2=_fieldName, op3=_fieldType)
class Set(Instruction):
    """
    Sets the field named *op2* in the struct referenced by *op1* to the value
    *op3*. The type of the *op3* must match the type of the field.
    """
    def _codegen(self, cg):
        (idx, ftype) = _getIndex(self)
        assert idx >= 0

        s = cg.llvmOp(self.op1())

        # Set mask bit.
        zero = cg.llvmGEPIdx(0)
        addr = cg.builder().gep(s, [zero, zero])
        mask = cg.builder().load(addr)
        bit = cg.llvmConstInt(1<<idx, 32)
        mask = cg.builder().or_(bit, mask)
        cg.llvmAssign(mask, addr)

        index = cg.llvmGEPIdx(idx + 1)
        addr = cg.builder().gep(s, [zero, index])
        cg.llvmAssign(cg.llvmOp(self.op3(), ftype), addr)


@hlt.instruction("struct.unset", op1=cReferenceOf(cStruct), op2=_fieldName)
class Unset(Instruction):
    """Unsets the field named *op2* in the struct referenced by *op1*. An
    unset field appears just as if it had never been assigned an value; in
    particular, it will be reset to its default value if has been one assigned.
    """
    def _codegen(self, cg):
        (idx, ftype) = _getIndex(self)
        assert idx >= 0

        s = cg.llvmOp(self.op1())

        # Clear mask bit.
        zero = cg.llvmGEPIdx(0)
        addr = cg.builder().gep(s, [zero, zero])
        mask = cg.builder().load(addr)
        bit = cg.llvmConstInt(~(1<<idx), 32)
        mask = cg.builder().and_(bit, mask)
        cg.llvmAssign(mask, addr)

@hlt.instruction("struct.is_set", op1=cReferenceOf(cStruct), op2=_fieldName, target=cBool)
class IsSet(Instruction):
    """Returns *True* if the field named *op2* has been set to a value, and
    *False otherwise. If the instruction returns *True*, a subsequent call to
    ~~Get will not raise an exception.
    """
    def _codegen(self, cg):
        (idx, ftype) = _getIndex(self)
        assert idx >= 0

        s = cg.llvmOp(self.op1())

        # Check mask.
        zero = cg.llvmGEPIdx(0)
        addr = cg.builder().gep(s, [zero, zero])
        mask = cg.builder().load(addr)

        bit = cg.llvmConstInt(1<<idx, 32)
        isset = cg.builder().and_(bit, mask)

        notzero = cg.builder().icmp(llvm.core.IPRED_NE, isset, cg.llvmConstInt(0, 32))
        cg.llvmStoreInTarget(self, notzero)

def _getIndex(instr):
    ty = instr.op1().type().refType()
    field = instr.op2().constant().value()
    try:
        (idx, i) = ty._idmap[field]
        return (idx, i.type())
    except IndexError:
        return (-1, None)
