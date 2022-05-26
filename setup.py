from distutils.core import setup, Extension
setup(
    name='mykmeanssp',
    author='Braa and Sara',
    version='1.0',
    description='kmeanscalc',
    ext_modules=[Extension('mykmeanssp', sources=['kmeans.c'])])
