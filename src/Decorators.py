def typecheck(*types):
    def check_types(f):
        def new_f(*args, **kwds):
            # remove self from args
            args_checks = args[1:]
            for (a, t) in zip(args_checks, types):
                assert isinstance(a, t), \
                    "arg %r does not match %s" % (a,t)
            return f(*args, **kwds)
        new_f.__name__ = f.__name__
        return new_f
    return check_types

##############################################################################

def valuecheck(*values):
    def check_values(f):
        def new_f(*args, **kwds):
            # remove self from args
            args_checks = args[1:]
            for arg_pos, (a, t) in enumerate(zip(args_checks, values)):
                assert a == t, "argument in position {arg_pos} does not match required value {a}, is {t} instead"\
                    .format(arg_pos=arg_pos+1, a=a, t=t)
            return f(*args, **kwds)
        new_f.__name__ = f.__name__
        return new_f
    return check_values

#EOF
