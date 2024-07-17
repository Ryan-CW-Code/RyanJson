import os
from building import *

# get current directory
cwd = GetCurrentDir()

# The set of source files associated with this SConscript file.
src = Glob('RyanJson/*.c')
path = [cwd + '/RyanJson']

group = DefineGroup('RyanJson', src, depend=[
                    "PKG_USING_RYAN_JSON"], CPPPATH=path)

Return('group')
