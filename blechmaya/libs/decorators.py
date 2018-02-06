class DebugHelper():
    """Helper class to print function name using 'with' pattern.
    """
    def __init__(self, funcName):
        self.funcName = funcName

    def __enter__(self):
        print "Enter: {0}".format(self.funcName)
        return self

    def __exit__(self, type, value, tb):
        print "Exit: {0}".format(self.funcName)

def func_debug_decorator(func):
    """
    @param func: function object
    @return: function wrapped with enter and exit methods
    """
    def func_wrapper(*args, **kwargs):
        with DebugHelper(func.__name__):
            return func(*args, **kwargs)
    return func_wrapper

