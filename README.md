MyPortal
========

A small sample of OpenGL to simulate Portal.

The project is a sample of some simple OpenGL commands that produces a a First 
Person effect. The scene generated is three rooms with hallways connecting them. 
In the large room, there are two glass walls on the far end, near the pit. On 
the other side of the glass walls are a few turrets and cubes. The scene consists 
of several colored spot lights and fog. Calculations are done on the current 
position of the user so that the user does not pass through the walls.

## How to compile:

To compile run:
  - $ make

This will generate an executable named final

##How to run:

To run the portal executable run:
  - $ ./portal

You can navigate throughout the scene by using the following keys:

Keys:
---------------------------------
    UP: walk forward
    DOWN: walk back
    LEFT: look left
    RIGHT: look right
    PG_UP: look up
    PG_DOWN: look down

To close the program use the 'esc' key.