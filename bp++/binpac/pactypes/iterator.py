# $Id$
#
# Generic iterator operators.

import binpac.type as type
import binpac.operator as operator

@operator.Deref(type.Iterator)
class _:
    def type(iter):
        return iter.type().derefType()

    def evaluate(cg, iter):
        tmp = cg.functionBuilder().addLocal("__elem", iter.type().derefType().hiltiType(cg))
        cg.builder().deref(tmp, iter.evaluate(cg))
        return tmp

@operator.IncrPrefix(type.Iterator)
class _:
    def type(iter):
        return iter.type()
    
    def evaluate(cg, iter):
        op = iter.evaluate(cg)
        cg.builder().incr(op, op)
        return op
    
@operator.IncrPostfix(type.Iterator)
class _:
    def type(iter):
        return iter.type()
    
    def evaluate(cg, iter):
        tmp = cg.functionBuilder().addLocal("__tmp", iter.type().hiltiType(cg))
        op = iter.evaluate(cg)
        cg.builder().assign(tmp, op)
        cg.builder().incr(op, op)
        return tmp

@operator.Equal(type.Iterator, type.Iterator)
class _:
    def type(iter1, iter2):
        return type.Bool()
    
    def evaluate(cg, iter1, iter2):
        tmp = cg.functionBuilder().addLocal("__equal", hilti.type.Bool())
        cg.builder().equal(tmp, iter1.evaluate(cg), iter2.evaluate(cg))
        return tmp
    
    
    
    
