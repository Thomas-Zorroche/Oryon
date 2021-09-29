# Oryon
User Interface Software based on the GLRenderer library

# Start Oryon
First clone GLRenderer.
```bash
git clone https://github.com/Thomas-Zorroche/GLRenderer.git
```
Then build GLRenderer and make sure there are .dll and .lib in install folder
```bash
cd GLRenderer
mkdir build && cd build
cmake ..
```
You need to have GLRenderer/install/Debug in your PATH environment variable.

Now clone Oryon and build it with this command
```bash
git clone https://github.com/Thomas-Zorroche/Oryon.git
cd Oryon
mkdir build && cd build
cmake .. -DCMAKE_GLRENDERER_DIR=absolute_path_of_GLRenderer_directory
```
Finally, you should be able to run Oryon with the GLRenderer library.


