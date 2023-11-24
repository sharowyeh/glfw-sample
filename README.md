# OpenGL/GLFW related projects #

vscode root directory involved opengl framework dependencies, mainly using for mac environment required compiler tool chain and options, refer to .vscode settings

for windows env, go to sample-app use .sln or .vcxproj instead

- issue(solved):
  updated: get imgui offical demo source code, found that mac env need further glfwWindowHint before init, to ensure opengl works properly

  sample-app not good in mac, null backend data exception occurred when calls opengl3 new frame, shows not done opengl3 init but did
  - getting GL3WglProc from libGL.dylib`glXXX is successful, but get gl versions via glGetIntegerv all return 0
  - init will not create backend renderer user data if check version failed during init
  - can not create new frame if there is no backend renderer user data
  - if ignore first time check version failure, the second time check version will get 2.1(in init following code block)
  - following get texture 2d via glGetIntegerv is 0
  - no rendering context on window screen
