##########
Compiling
##########

C++17 is necessary for :code:`filesystem`, so make sure to specify an up to
date compiler. The code base is tested against :code:`g++-9`.

Following dependencies are used:

- `GLFW <https://www.glfw.org/download.html>`_ 
- `glad <https://glad.dav1d.de/>`_
- `stb_image.h <https://github.com/nothings/stb>`_

Check the CMakeList.txt file for their assumed locations.

All headers go under :code:`include`, some custom headers are provided for
having basic functionality like a moving camera, lights etc.
