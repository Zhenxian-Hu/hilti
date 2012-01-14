/// \type Integer
///
/// The *integer* data type represents signed and unsigned integers of a
/// fixed width. The width is specified as parameter to the type, e.g., 
/// ``int<16>`` for a 16-bit integer.
///
/// Note that HILTI has only a single data type representing both signed and
/// unsigned integer values. For instructions where signedness matters, there
/// are always two versions differing only in how they interpret their
/// arguments (e.g., there's ``int.sleq`` and ``int.uleq`` doing
/// lower-or-equal comparision for signed and unsigned operands,
/// respectively.
///
/// If not explictly initialized, integers are set to zero initially.
///
/// \ctor 42, -10
///
/// \cproto An ``int<n>`` is mapped to C integers depending on its width *n*,
/// per the following table:
///
///     ======  =======
///     Width   C type
///     ------  -------
///     1..8    int8_t
///     9..16   int16_t
///     17..32  int32_t
///     33..64  int64_t
///     ======  =======

iBegin(integer, Add, "int.add")
    iTarget(type::Integer)
    iOp1(type::Integer, true)
    iOp2(type::Integer, true)

    iValidate {
        canCoerceTo(op1, target);
        canCoerceTo(op2, target);
    }

    iDoc(R"(    
        Calculates the sum of the two operands. Operands and target must be of
        same width. The result is calculated modulo 2^{width}.
     )")

iEnd


iBegin(integer, Sub, "int.sub")
    iTarget(type::Integer)
    iOp1(type::Integer, true)
    iOp2(type::Integer, true)

    iValidate {
        canCoerceTo(op1, target);
        canCoerceTo(op2, target);
    }

    iDoc(R"(    
        Subtracts *op2* from *op1*. Operands and target must be of same width.
        The result is calculated modulo 2^{width}.
     )")

iEnd

iBegin(integer, Div, "int.div")
    iTarget(type::Integer)
    iOp1(type::Integer, true)
    iOp2(type::Integer, true)

    iValidate {
        canCoerceTo(op1, target);
        canCoerceTo(op2, target);

        auto c = ast::as<expression::Constant>(op2);

        if ( c ) {
            auto i = ast::as<constant::Integer>(c->constant());
            assert(i);

            if ( i->value() == 0 )
                error(op2, "Division by zero");
        }
    }

    iDoc(R"(    
        Divides *op1* by *op2*, flooring the result. Operands and target must
        be of same width.  If the product overflows the range of the integer
        type, the result in undefined.  Throws :exc:`DivisionByZero` if *op2*
        is zero.
     )")

iEnd

#if 0

iBegin(integer, Sleq, "int.sleq")
    iTarget(bool)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<bool>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Returns true iff *op1* is lower or equal *op2*, interpreting both as
        *signed* integers.
     )")

iEnd

iBegin(integer, AsSDouble, "int.as_sdouble")
    iTarget(double)
    iOp1(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<double>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
    }

    iDoc(R"(    
        Converts the signed integer *op1* into a double value.
     )")

iEnd

iBegin(integer, Pow, "int.pow")
    iTarget(type::Integer)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<type::Integer>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Raise *op1* to the power *op2*. Note that both *op1* and *op2* are
        interpreted as unsigned integers. If the result overflows the target's
        type, the result is undefined.
     )")

iEnd

iBegin(integer, SExt, "int.sext")
    iTarget(type::Integer)
    iOp1(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<type::Integer>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
    }

    iDoc(R"(    
        Sign-extends *op1* into an integer of the same width as the *target*.
        The width of *op1* must be smaller or equal that of the *target*.
     )")

iEnd

iBegin(integer, Shr, "int.shr")
    iTarget(type::Integer)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<type::Integer>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Shifts *op1* to the right by *op2* bits. The most-signficant bits are
        filled with zeros. If the value of *op2* is larger than the integer
        type has bits, the result is undefined.
     )")

iEnd

iBegin(integer, Mul, "int.mul")
    iTarget(type::Integer)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<type::Integer>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Multiplies *op1* with *op2*. Operands and target must be of same
        width. The result is calculated modulo 2^{width}.
     )")

iEnd

iBegin(integer, Shl, "int.shl")
    iTarget(type::Integer)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<type::Integer>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Shifts *op1* to the left by *op2* bits. The least-signficant bits are
        filled with zeros. If the value of *op2* is larger than the integer
        type has bits, the result is undefined.
     )")

iEnd

iBegin(integer, Ult, "int.ult")
    iTarget(bool)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<bool>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Returns true iff *op1* is less than *op2*, interpreting both as
        *unsigned* integers.
     )")

iEnd

iBegin(integer, Uleq, "int.uleq")
    iTarget(bool)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<bool>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Returns true iff *op1* is lower or equal *op2*, interpreting both as
        *unsigned* integers.
     )")

iEnd

iBegin(integer, Ashr, "int.ashr")
    iTarget(type::Integer)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<type::Integer>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Arithmetically shifts *op1* to the right by *op2* bits. The most-
        signficant bits are filled with the sign of *op1*. If the value of
        *op2* is larger than the integer type has bits, the result is
        undefined.
     )")

iEnd

iBegin(integer, Mask, "int.mask")
    iTarget(type::Integer)
    iOp1(type::Integer, CONST?)
    iOp2(int\ <64>, CONST?)
    iOp3(int\ <64>, CONST?)

    iValidate {
        auto ty_target = as<type::Integer>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<int\ <64>>(op2->type());
        auto ty_op3 = as<int\ <64>>(op3->type());
    }

    iDoc(R"(    
        Extracts the bits *op2* to *op3* (inclusive) from *op1* and shifts
        them so that they align with the least significant bit in the result.
     )")

iEnd

iBegin(integer, Mod, "int.mod")
    iTarget(type::Integer)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<type::Integer>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Calculates the remainder of dividing *op1* by *op2*. Operands and
        target must be of same width.  Throws :exc:`DivisionByZero` if *op2*
        is zero.
     )")

iEnd

iBegin(integer, Eq, "int.eq")
    iTarget(bool)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<bool>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Returns true iff *op1* equals *op2*.
     )")

iEnd

iBegin(integer, ZExt, "int.zext")
    iTarget(type::Integer)
    iOp1(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<type::Integer>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
    }

    iDoc(R"(    
        Zero-extends *op1* into an integer of the same width as the *target*.
        The width of *op1* must be smaller or equal that of the *target*.
     )")

iEnd

iBegin(integer, AsTime, "int.as_time")
    iTarget(time)
    iOp1(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<time>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
    }

    iDoc(R"(    
        Converts the integer *op1* into a time value, interpreting it as
        seconds since the epoch.
     )")

iEnd

iBegin(integer, Slt, "int.slt")
    iTarget(bool)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<bool>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Returns true iff *op1* is less than *op2*, interpreting both as
        *signed* integers.
     )")

iEnd

iBegin(integer, Trunc, "int.trunc")
    iTarget(type::Integer)
    iOp1(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<type::Integer>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
    }

    iDoc(R"(    
        Bit-truncates *op1* into an integer of the same width as the *target*.
        The width of *op1* must be larger or equal that of the *target*.
     )")

iEnd

iBegin(integer, Sqeq, "int.sgeq")
    iTarget(bool)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<bool>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Returns true iff *op1* is greater or equal *op2*, interpreting both as
        *signed* integers.
     )")

iEnd

iBegin(integer, AsUDouble, "int.as_udouble")
    iTarget(double)
    iOp1(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<double>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
    }

    iDoc(R"(    
        Converts the unsigned integer *op1* into a double value.
     )")

iEnd

iBegin(integer, Or, "int.or")
    iTarget(type::Integer)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<type::Integer>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Calculates the binary *or* of the two operands. Operands and target
        must be of same width.
     )")

iEnd

iBegin(integer, Ugeq, "int.ugeq")
    iTarget(bool)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<bool>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Returns true iff *op1* is greater or equal *op2*, interpreting both as
        *unsigned* integers.
     )")

iEnd

iBegin(integer, Sgt, "int.sgt")
    iTarget(bool)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<bool>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Returns true iff *op1* is greater than *op2*, interpreting both as
        *signed* integers.
     )")

iEnd

iBegin(integer, AsDouble, "int.as_interval")
    iTarget(interval)
    iOp1(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<interval>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
    }

    iDoc(R"(    
        Converts the integer *op1* into an interval value, interpreting it as
        seconds.
     )")

iEnd

iBegin(integer, Xor, "int.xor")
    iTarget(type::Integer)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<type::Integer>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Calculates the binary *xor* of the two operands. Operands and target
        must be of same width.
     )")

iEnd

iBegin(integer, And, "int.and")
    iTarget(type::Integer)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<type::Integer>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Calculates the binary *and* of the two operands. Operands and target
        must be of same width.
     )")

iEnd

iBegin(integer, Ugt, "int.ugt")
    iTarget(bool)
    iOp1(type::Integer, CONST?)
    iOp2(type::Integer, CONST?)

    iValidate {
        auto ty_target = as<bool>(target->type());
        auto ty_op1 = as<type::Integer>(op1->type());
        auto ty_op2 = as<type::Integer>(op2->type());
    }

    iDoc(R"(    
        Returns true iff *op1* is greater than *op2*, interpreting both as
        *unsigned* integers.
     )")

iEnd


#endif

