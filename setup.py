from distutils.core import setup, Extension

atomicint_mod = Extension('atomicint',
                          sources=['atomicint.c'])

setup(name='atomicint',
      version='1.0',
      description='Atomic Integer',
      ext_modules=[atomicint_mod])
