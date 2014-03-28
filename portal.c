#include "mgltools.h"
#include <unistd.h>     // Header file for sleeping.
#include <math.h>

#define PI 3.1415926535897932384626433832795

float op =  0.0, po =  0.0, zo = 0.0, qo=0.0;

unsigned int texture[4]; // Texture names
int min_el = -10;
int max_el =  190;
//  Field of view (for perspective)
int fov=50;
// Display axes
int axes=0;
// Aspect ratio
double asp=1;
// Size of the world
double dim=5.0;

int emission  =   0;  // Emission intensity (%)
float shinyvec[1];    // Shininess (value)

GLfloat yrot =  -49.0;            // y rotation

GLfloat lookupdown = 0.0;
const float radians = 0.0174532925f;

float xpos = 10.7, zpos =  17.9;

GLfloat z=0.0f;                       // depth into the screen.

int num_sectors = 17;
float sectors[][8] = {
    //   max_x  min_x  max_z  min_z    L    R    F    B
    {    9.0,   1.8,   9.0,   0.0,    1.0, 0.0, 1.0, 0.0 }, // rm1 F/L
    {   -2.0,  -9.0,   9.0,   0.0,    0.0, 1.0, 1.0, 0.0 }, // rm1 F/R
    {   -3.8,  -9.0,   0.0,  -7.5,    0.0, 1.0, 0.0, 1.0 }, // rm1 B/R
    {    9.0,   3.7,   0.0,  -7.5,    1.0, 0.0, 0.0, 1.0 }, // rm1 B/L
    {    1.5,  -1.5,  17.8,  10.0,    1.0, 1.0, 0.0, 0.0 }, // hw1 
    {    9.0,   2.0,  37.0,  19.0,    1.0, 0.0, 1.0, 1.0 }, // rm2 B/l
    {    2.0,  -8.0,  37.0,  33.0,    0.0, 0.0, 1.0, 0.0 }, // rm2 Mid
    {   -8.0,  -9.0,  37.0,  33.0,    0.0, 1.0, 1.0, 1.0 }, // rm2 F/R
    {   -1.5,  -7.0,  33.0,  24.0,    0.0, 1.0, 0.0, 0.0 }, // rm2 M/R
    {   -1.5,  -8.0,  24.0,  19.0,    0.0, 0.0, 0.0, 1.0 }, // rm2 B/R
    {   -8.0,  -9.0,  23.0,  19.0,    0.0, 1.0, 1.0, 1.0 },  // rm2 M/B

    {   -13.0,  -38.0,  28.0, 24.0,    1.0, 1.0, 1.0, 0.0 }, // rmr3 L 
    {     3.0,  -38.0, -12.0,-18.0,    1.0, 1.0, 0.0, 1.0 },  // rmr3 R 
    {   -11.0,  -12.0,  23.0,-12.0,    1.0, 0.0, 1.0, 0.0 },  // rmr3 B 
    {   -34.0,  -38.0,  24.0,-12.0,    0.0, 1.0, 0.0, 0.0 }, // rmr3 F 

    {     3.0,   -3.0,  -9.0,-12.0,    1.0, 1.0, 0.0, 0.0 },  // hw2
    {    -3.0,  -11.0, -13.0,-15.0,    0.0, 0.0, 1.0, 0.0 }  // hw2
};

int in_sector(double x, double z, float sector[4]) {
    float tol = 0.4;
    if (x <= sector[0]+tol && x >= sector[1]-tol &&
        z <= sector[2]+tol && z >= sector[3]-tol) {
        return 1; 
    }

    return 0;
}

/*
 * Calcuate the location of the camera for First Person effect.
 */
void camera() {
    GLfloat xtrans, ztrans, ytrans, sceneroty;

    // calculate translations and rotations.
    xtrans = -xpos;
    ztrans = -zpos;
    ytrans = -0.25f;
    sceneroty = 360.0f - yrot;

    glRotatef(lookupdown, 1.0f, 0, 0);
    glRotatef(sceneroty, 0, 1.0f, 0);

    // Check the current position
    // Does it collide with a wall
    int i, xset = 0, zset = 0;
    for (i = 0; i < num_sectors; i++) {
        if (in_sector(xtrans, ztrans, sectors[i])) {
            if (xtrans >= sectors[i][0] && sectors[i][4] > 0) {
                xtrans = sectors[i][0];
                xpos = -xtrans;
                xset = 1;
            } else if (xtrans <= sectors[i][1] && sectors[i][5] > 0) {
                xtrans = sectors[i][1];
                xpos = -xtrans;
                xset = 1;
            }

            if (ztrans >= sectors[i][2] && sectors[i][6] > 0.0) {
                ztrans = sectors[i][2];
                zpos = -ztrans;
                zset = 1;
            } else if (ztrans <= sectors[i][3] && sectors[i][7] > 0) {
                ztrans = sectors[i][3];
                zpos = -ztrans;
                zset = 1;
            }

            if (xset || zset)
                break;
        }
    }

    glTranslatef(xtrans, ytrans, ztrans);
}

/*
 * Convenience routine to output raster text
 *
 * Taken from class examples.
 */
#define LEN 8192  //  Maximum length of text string
void Print(const char* format , ...)
{
    char    buf[LEN];
    char*   ch=buf;
    va_list args;
    //  Turn the parameters into a character string
    va_start(args,format);
    vsnprintf(buf,LEN,format,args);
    va_end(args);
    //  Display the characters one at a time at the current raster position
    while (*ch)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*ch++);
}

/*
 * Draws the axes for X, Y, Z
 */
void drawAxes() {
    const double len=3.0;
    
    //  White
    glColor3f(1,1,1);
    //  Draw axes
    glBegin(GL_LINES);
    glVertex3d(0.0,0.0,0.0);
    glVertex3d(len,0.0,0.0);
    glVertex3d(0.0,0.0,0.0);
    glVertex3d(0.0,len,0.0);
    glVertex3d(0.0,0.0,0.0);
    glVertex3d(0.0,0.0,len);
    glEnd();

    //  Label axes
    glRasterPos3d(len,0.0,0.0);
    Print("X");
    glRasterPos3d(0.0,len,0.0);
    Print("Y");
    glRasterPos3d(0.0,0.0,len);
    Print("Z");

}


/*
 * Draws a side for the corner cube. 
 *
 */
void drawCornerSide(float color[], int normal[], int side[]) {
    glColor3f(color[0], color[1], color[2]);
    glNormal3f(normal[0], normal[1], normal[2]);

    int i;    
    for (i=0; i < 24; i += 3) {
        glVertex3f(side[i], side[i+1], side[i+2]);
    }
}

/*
 * Draws a corner cube for the companion cube
 */
void corner_cube(double x,double y,double z,
                 double dx,double dy,double dz,
                 double th, double xh, double yh, double zh) {
    //  Save transformation
    glPushMatrix();

    //  Offset
    glTranslated(x,y,z);
    glRotated(th,xh,yh,zh);
    glScaled(dx,dy,dz);

    int num_sides = 6;
    int i;

    float colors[3] = {0.7, 0.7, 0.7};
    int normals[6][3] = {
        { 0, 0, 1 }, // Front
        { 1, 0, 0 }, // Back
        { 0, 0, 0 }, // Middle
        {-1, 0, 0 }, // Left
        { 1, 0, 0 }, // Right
        { 0, 1, 1 }  // Top
    };

    int sides[6][24] = {
        { -1, 1,  1,    -1, 0,  1,    1, 0,  1,    1,  1,  1,  -1, 0,  1,   0, -1, 1,   1, -1, 1,     1,  0,  1},  // Front
        {  0, 1, -1,     0, 0, -1,    1, 0, -1,    1,  1, -1,   0, 0, -1,   0, -1, 0,   1, -1, 0,     1,  0, -1},  // Back
        {  0, 0, -1,    -1, 0,  0,   -1, 1,  0,    0,  1, -1,  -1, 0,  0,   0,  0,-1,   0, -1, 0,     0, -1,  0},  // Middle
        { -1, 0,  0,    -1, 0,  1,   -1, 1,  1,   -1,  1,  0,   0,-1,  0,   0, -1, 1,  -1,  0, 1,    -1,  0,  0},  // Left
        {  1, 0,  1,     1, 0, -1,    1,-1, -0,    1, -1,  1,   1,-0,  1,   1, -0,-1,   1,  1,-1,     1,  1,  1},  // Right
        {  0, 1, -1,     1, 1, -1,    1, 1,  1,    0,  1,  1,   0, 1,  1,  -1,  1, 1,  -1,  1, 0,     0,  1, -1}   // Top
    };

    //  Cube
    glBegin(GL_QUADS);

    for (i=0;i<num_sides;i++) {
        drawCornerSide(colors, normals[i], sides[i]);
    }

    //  Bottom
    glColor3f(0.39,0.54,0.6);
    glVertex3f(0,-1,0);
    glVertex3f(0,-1,+1);
    glVertex3f(+1,-1,+1);
    glVertex3f(+1, -1,0);

    //  End
    glEnd();
    //  Undo transformations
    glPopMatrix();
}

/*
 * Draws a square.
 *
 */
void drawSquare(float color[], int side[]) {
    glColor3f(  color[0],  color[1],  color[2]);
    glNormal3f( side[0],  side[1],  side[2]);

    glTexCoord2f(0.0, 0.0);  glVertex3f( side[3],  side[4],  side[5]);
    glTexCoord2f(1.0, 0.0);  glVertex3f( side[6],  side[7],  side[8]);
    glTexCoord2f(1.0, 1.0);  glVertex3f( side[9],  side[10], side[11]);
    glTexCoord2f(0.0, 1.0);  glVertex3f( side[12], side[13], side[14]);
}

/*
 * Draws a cube.
 *
 */
void cube(double x,double y,double z,
                 double dx,double dy,double dz,
                 double th) {
    //  Save transformation
    glPushMatrix();
    //  Offset, scale and rotate
    glTranslated(x,y,z);
    glRotated(th,0,1,0);
    glScaled(dx,dy,dz);

    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,texture[1]);

    int i;
    int sides[6][18] = {
        // Normal                       Sides
        { 0, 0, 1,   -1,-1, 1,   1,-1, 1,   1, 1, 1,  -1, 1, 1}, // Front
        { 0, 0,-1,    1,-1,-1,  -1,-1,-1,  -1, 1,-1,   1, 1,-1}, // Back
        { 1, 0, 0,    1,-1, 1,   1,-1,-1,   1, 1,-1,   1, 1, 1}, // Right
        {-1, 0, 0,   -1,-1,-1,  -1,-1, 1,  -1, 1, 1,  -1, 1,-1}, // Left
        { 0, 1, 1,   -1, 1, 1,   1, 1, 1,   1, 1,-1,  -1, 1,-1}, // Top
        { 0,-1, 0,   -1,-1,-1,   1,-1,-1,   1,-1, 1,  -1,-1, 1}  // Botto,
    };  

    float colors[] = {1, 1, 1};

    //  Cube
    glBegin(GL_QUADS);

    for (i=0; i<6; i++) {
        drawSquare(colors, sides[i]);
    }

    //  End
    glEnd();
    //  Undo transofrmations
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

/*
 * Draw vertex in polar coordinates
 *
 * Uses a grey scale for coloring.
 */
static void Vertex(double th,double ph) {

   double x =  Sin(th)*Cos(ph);
   double y =  Cos(th)*Cos(ph);
   double z =          Sin(ph);
   //  For a sphere at the origin, the position
   //  and normal vectors are the same
   glNormal3d(x,y,z);

    glColor3f(1, 1, 1);
    glTexCoord2d(th/360.0,ph/180.0+0.5);
    glVertex3d(x,y,z);
}

/*
 * Draws a sphere at (x,y,z) with radius r
 *
 */
void sphere(double x,double y,double z, double r) {
    const int d=5;
    int th,ph;
    float yellow[] = {1.0,1.0,0.0,1.0};
    float Emission[]  = {0.0,0.0,0.01*emission,1.0};

    //  Save transformation
    glPushMatrix();
    //  Offset and scale
    glTranslated(x,y,z);
    glScaled(r,r,r);

    glMaterialfv(GL_FRONT,GL_SHININESS,shinyvec);
    glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
    glMaterialfv(GL_FRONT,GL_EMISSION,Emission);

    //  Latitude bands
    for (ph=-90;ph<90;ph+=d) {
        glBegin(GL_QUAD_STRIP);
        for (th=0;th<=360;th+=d) {
            Vertex(th,ph);
            Vertex(th,ph+d);
        }
        glEnd();
    }

    //  Undo transformations
    glPopMatrix();
}

/* 
 * Creates the light source for the scene.
 *
 */
void createLight(float lx, float ly, float lz, float ambient, float diffuse, float specular, GLenum light) {
    //  Translate intensity to color vectors
    float Ambient[]   = {ambient ,ambient ,ambient ,1.0};
    float Diffuse[]   = {diffuse ,diffuse ,diffuse ,1.0};
    float Specular[]  = {specular,specular,specular,1.0};

    //  Light position
    float Position[]  = {lx,ly,lz,1.0};

    //  Draw light position as ball (still no lighting here)
    glColor3f(1,1,1);
    sphere(Position[0],Position[1],Position[2] , 0.1);

    glEnable(GL_COLOR_MATERIAL);

    //  Enable light
    glEnable(light);

    //  Set ambient, diffuse, specular components and position of light 0
    glLightfv(light,GL_AMBIENT ,Ambient);
    glLightfv(light,GL_DIFFUSE ,Diffuse);
    glLightfv(light,GL_SPECULAR,Specular);
    glLightfv(light,GL_POSITION,Position);
}

void createSpotLight(float lx,      float ly,      float lz, 
                     float r,       float g,       float b,
                     float dx,      float dy,      float dz,    
                     float sco,     float exp,     GLenum light) {
    //  Enable light
    glEnable(light);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);

    float Direction[] = {dx, dy, dz};
    //  Light position
    float Position[]  = {lx, ly, lz, 1.0};
    float spec[] = { r, g, b, 1.0 };
    float shinyvec[]={ 50.0 };

    glClearColor (0.5, 0.5, 0.5, 0.0);
    glShadeModel(GL_SMOOTH);

    //  Draw light position as ball (still no lighting here)
    glColor3f(1,1,1);
    sphere(Position[0],Position[1],Position[2] , 0.1);

    //  OpenGL should normalize normal vectors
    glEnable(GL_NORMALIZE);

    glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glLightfv(light,GL_SPECULAR, spec);
    glLightfv(light,GL_POSITION,Position);

    //  Set specular colors
    glMaterialfv(GL_FRONT,GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT,GL_SHININESS,shinyvec);

    glLightf(light, GL_SPOT_EXPONENT,  exp);
    glLightfv(light,GL_SPOT_DIRECTION, Direction);
    glLightf(light, GL_SPOT_CUTOFF,    sco);
}

/*
 * Draws a wall using the concrete texture.
 *
 */
void drawWall(double x, double y, double z, 
            double th, double xh, double yh, double zh, // rotation params
            double nx, double ny, double nz) { // normal params
    // Save Transformation
    glPushMatrix();

    glTranslated(x, y, z);
    glRotated(th,xh,yh,zh);

    // Enable texture
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,texture[0]);

    int i, j;
    int num = 20;
    double mul = 2.0/num;
    int z_index = 0;
    int size = 2;

    glColor3f(0.5, 0.5, 0.5);
    glNormal3f(nx,ny,nz);
    glBegin(GL_QUADS);
    for (i=0;i<num;i++) { // width
        for (j=0;j<num;j++) { // height
            glTexCoord2d(mul*(i+0),mul*(j+0)); glVertex3d(size*mul*(i+0)-size,size*mul*(j+0)-size, z_index);
            glTexCoord2d(mul*(i+1),mul*(j+0)); glVertex3d(size*mul*(i+1)-size,size*mul*(j+0)-size, z_index);
            glTexCoord2d(mul*(i+1),mul*(j+1)); glVertex3d(size*mul*(i+1)-size,size*mul*(j+1)-size, z_index);
            glTexCoord2d(mul*(i+0),mul*(j+1)); glVertex3d(size*mul*(i+0)-size,size*mul*(j+1)-size, z_index);
        }
    }
    glEnd();
    // Disable texture
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

void drawGlass(double x, double y, double z, 
            double th, double xh, double yh, double zh, // rotation params
            double sx, double sy, double sz, // scale params
            double nx, double ny) { // normal params
    // Save Transformation
    glPushMatrix();

    glTranslated(x, y, z);
    glRotated(th,xh,yh,zh);
    glScaled(sx, sy, sz);

    // Enable texture
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,texture[2]);

    float colors[3] = {1, 1, 1};
    int sides[15] =  { 0, 0, 1,   -1,-1, 1,   1,-1, 1,   1, 1, 1,  -1, 1, 1};

    glBegin(GL_QUADS);

    drawSquare(colors, sides);

    //  End
    glEnd();

    // Disable texture
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

/* 
 * Draws the floor for the room
 */
void drawFloor(int offset, int y, float yn) {
    float i, j;
    for (i = -8.0; i < 12; i+=4) {
        for (j = -8.0; j < 12; j+=4) {
            drawWall(i, y, j+offset, 90, 0, 0, 0, 0, 0, yn);
        }
    }
}

/*
 * Draws a companion cube.
 */
void companionCube(double x, double y, double z, double scale) {
    double c_sides = scale - (scale * 0.1);

    cube(x,y,z,  c_sides,c_sides,c_sides, 0);

    float c_pct = 0.35;
    float c_size = scale * c_pct;
    float offset = .66 * scale - 0.03145;

    corner_cube( x + offset, y + offset, z + offset,    c_size,c_size,c_size,      0,    0, 1,   0); // Front-Top-Right
    corner_cube( x + offset, y + offset, z - offset,    c_size,c_size,c_size,     90,    0, 1,   0); // Back-Top-Right
    corner_cube( x - offset, y + offset, z - offset,    c_size,c_size,c_size,    180,    0, 1,   0); // Back-Top-Left
    corner_cube( x - offset, y + offset, z + offset,    c_size,c_size,c_size,    -90,    0, 1,   0); // Front-Top-Left

    corner_cube( x + offset, y - offset, z + offset,    c_size,c_size,c_size,     90,  180, 1,   0);
    corner_cube( x + offset, y - offset, z - offset,    c_size,c_size,c_size,    180,  180, 1,   0);
    corner_cube( x - offset, y - offset, z - offset,    c_size,c_size,c_size,    180, -180, 1, 180);
    corner_cube( x - offset, y - offset, z + offset,    c_size,c_size,c_size,    180,    0, 1, 180);

}

void drawTurretBody(double x, double y, double z, double fh, double xh, double yh, double zh) {
    const int d=5;
    int th, ph;

    //  Save transformation
    glPushMatrix();

    glTranslated(x, y, z);
    glRotated(fh, xh, yh, zh);
    glScaled(0.5, 0.8, 1.5);

    glEnable(GL_TEXTURE_2D);
    //glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,texture[3]);

    for (ph=-90;ph<90;ph+=d) {
        glBegin(GL_QUAD_STRIP);
        for (th=0;th<=360;th+=d) {
            Vertex(th,ph);
            Vertex(th,ph+d);
        }
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);

    //  Undo transformations
    glPopMatrix();
}

void drawCover(double x,  double y,  double z, 
               double fh, double xh, double yh, double zh,      
               double rx, double ry, double rz) {
    glPushMatrix();

    glTranslated(x, y, z);
    glRotated(fh, xh, yh, zh);
    glScaled(rx, ry, rz);

    glColor3f(1,1,1); 

    glNormal3f(0, 1, 0);
    glBegin(GL_QUADS);

    // Back
    glVertex3d(-0.10, 0.25, -0.25);  
    glVertex3d( 0.10, 0.25, -0.25);  
    glVertex3d( 0.07, 0.15, -0.35);
    glVertex3d(-0.07, 0.15, -0.35);

    glVertex3d(-0.07, 0.15, -0.35);
    glVertex3d( 0.07, 0.15, -0.35);
    glVertex3d( 0.05, 0.00, -0.40);
    glVertex3d(-0.05, 0.00, -0.40);

    // Front
    //glNormal3f(0.5, 0.1, -0.1);
    glNormal3f(0.1, -0.2, 0.2);
    glVertex3d(-0.10, 0.25, -0.25);  
    glVertex3d( 0.10, 0.25, -0.25);  
    glVertex3d( 0.05, 0.15, 0.20);
    glVertex3d(-0.05, 0.15, 0.20);

    glNormal3f(0.1, -0.3, 0.2);
    glVertex3d(-0.05, 0.15, 0.20);  
    glVertex3d( 0.05, 0.15, 0.20);  
    glVertex3d( 0.02, -0.05, 0.30);
    glVertex3d(-0.02, -0.05, 0.30);

    // Right
    glNormal3f(0.5, -1.0, 1.0);
    glVertex3d( 0.05, 0.15, 0.20);  
    glVertex3d( 0.12, 0.00, 0.15);  
    glVertex3d( 0.10,-0.04, 0.20);  
    glVertex3d( 0.02,-0.05, 0.30);  

    glNormal3f(0.2, -0.3, 1.0);
    glVertex3d( 0.05, 0.15, 0.20);  
    glVertex3d( 0.12, 0.00, 0.15);  
    glVertex3d( 0.15, 0.0, -0.25);  
    glVertex3d( 0.10, 0.25, -0.25);  

    glNormal3f(0.2, -0.4, 1.0);
    glVertex3d( 0.15, 0.0, -0.25);  
    glVertex3d( 0.10, 0.25, -0.25);  
    glVertex3d( 0.07, 0.15, -0.35);  
    glVertex3d( 0.15, 0.00, -0.25);  

    glNormal3f(0.2, -0.4, 1.0);
    glVertex3d( 0.07, 0.15, -0.35);  
    glVertex3d( 0.15, 0.00, -0.25);  
    glVertex3d( 0.05, 0.00, -0.40);  
    glVertex3d( 0.05, 0.00, -0.40);  


    // Left
    glNormal3f(0.5, -1.0, 1.0);
    glVertex3d( -0.05, 0.15, 0.20);  
    glVertex3d( -0.12, 0.00, 0.15);  
    glVertex3d( -0.10,-0.04, 0.20);  
    glVertex3d( -0.02,-0.05, 0.30);  

    glNormal3f(0.2, -0.3, 1.0);
    glVertex3d( -0.05, 0.15, 0.20);  
    glVertex3d( -0.12, 0.00, 0.15);  
    glVertex3d( -0.15, 0.0, -0.25);  
    glVertex3d( -0.10, 0.25, -0.25);  

    glNormal3f(0.2, -0.4, 1.0);
    glVertex3d( -0.15, 0.0, -0.25);  
    glVertex3d( -0.10, 0.25, -0.25);  
    glVertex3d( -0.07, 0.15, -0.35);  
    glVertex3d( -0.15, 0.00, -0.25);  

    glNormal3f(0.2, -0.4, 1.0);
    glVertex3d( -0.07, 0.15, -0.35);  
    glVertex3d( -0.15, 0.00, -0.25);  
    glVertex3d( -0.05, 0.00, -0.40);  
    glVertex3d( -0.05, 0.00, -0.40);  

    glEnd();

    glPopMatrix();
}

void drawTurretLeg(double x, double y, double z, double fh, double xh, double yh, double zh) {
    //  Save transformation
    glPushMatrix();

    //op, po, zo, qo=90.0;
    glTranslated(x, y, z);
    glRotated(fh, xh, yh, zh);
    glScaled(1, 1, 1);

    double wid =  0.05;
    double toe =  0.0;
    double bot = -0.75;
    double top =  0.25;

    //draw legs
    glColor3f(0,0,0);
    glBegin(GL_QUADS);

    glVertex3d(wid, top, wid);
    glVertex3d(wid, top, -wid);
    glVertex3d(wid, bot, -wid);
    glVertex3d(wid, bot, wid);

    glVertex3d(-wid, top, wid);
    glVertex3d(-wid, top, -wid);
    glVertex3d(-wid, bot, -wid);
    glVertex3d(-wid, bot, wid);

    glVertex3d(-wid, top, wid);
    glVertex3d( wid, top, wid);
    glVertex3d( wid, bot, wid);
    glVertex3d(-wid, bot, wid);

    glVertex3d(-wid, top,-wid);
    glVertex3d( wid, top,-wid);
    glVertex3d( wid, bot,-wid);
    glVertex3d(-wid, bot,-wid);
    
    glEnd();

    glBegin(GL_TRIANGLES);

    glVertex3d(toe, -1.0, 0.0);
    glVertex3d( wid, bot,  wid);
    glVertex3d( wid, bot, -wid);

    glVertex3d( toe, -1.0, 0.0);
    glVertex3d(  wid,  bot,-wid);
    glVertex3d( -wid,  bot,-wid);

    glVertex3d(toe, -1.0, 0.0);
    glVertex3d( wid,  bot, wid);
    glVertex3d(-wid,  bot, wid);

    glVertex3d(toe, -1.0, 0.0);
    glVertex3d(-wid,  bot, wid);
    glVertex3d(-wid,  bot,-wid);

    glEnd();


    //  Undo transformations
    glPopMatrix();
}


void drawTurret(double x, double y, double z, double zh) {
    drawTurretBody(x, y, z, 102.9, 22.6, 12.8, -12.4);
    drawTurretLeg(x-1.1, -1, z-1.5, 14.0, 0.0, 0.0, 0.0);
    // Draw Leg Cover
    drawCover(x-0.85, -0.9, z-1.15, -148.8, 0.0, 21.6, 1.9, 1.7, 1.6, 2.0);
    // Draw Leg Cover
    drawCover(x-1.40, -0.9, z-0.25, -104.1, 1.9, 21.6, 1.9, 1.7, 1.6, 2.0);
    // Draw Leg Cover
    drawCover(x+1.2, -0.8, z+0.65, 61.5, -3.3, 22.2, 1.9, 2.0, 1.8, 1.8);

    drawTurretLeg(x-1.8, -1, z-0.3, 14.0, -7.0, -10.0, -33.0);
    drawTurretLeg(x+1.5, -1, z+0.9, 0.0, -4.6, -2.2, -3.6);
}

/*
 * Draws the walls for the room.
 *
 */
void drawRoom1(int room_size, int room[4][2]) {
    int i;
    int j;
    
    drawWall(4, 0, room_size, room[3][0], 0, 1, 0, 0, 0, 1);

    // Draw back wall
    // Draw lower open wall
    drawWall(6, 0, room[0][1]+11, room[0][0], 0, 1, 0, 0, 0, -1);
    drawWall(8, 0, room[0][1]+11, room[0][0], 0, 1, 0, 0, 0, -1);

    // back right corner wall
    for (j = 0; j <= 8; j += 4) {
        drawWall(-8, j, room[0][1]+7, room[0][0], 0, 1, 0, 0, 0, -1);
        drawWall(-6, j, room[0][1]+7, room[0][0], 0, 1, 0, 0, 0, -1);
    }

    // back left corner wall
    for (j = 0; j <= 8; j += 4) {
        drawWall(8, j, room[0][1]+7, room[0][0], 0, 1, 0, 0, 0, -1);
        drawWall(6, j, room[0][1]+7, room[0][0], 0, 1, 0, 0, 0, -1);
    }

    // Draw front wall
    // Draw lower open wall
    drawWall(-8, 0, room[1][1]*room_size, room[1][0],  0,   1,   0, 0, 0, -1);
    drawWall(-4, 0, room[1][1]*room_size, room[1][0],  0,   1,   0, 0, 0, -1);
    drawWall(4, 0, room[1][1]*room_size, room[1][0],  0,   1,   0, 0, 0, -1);
    drawWall(8, 0, room[1][1]*room_size, room[1][0],  0,   1,   0, 0, 0, -1);

    for (j = -8; j <= 10; j += 4) {
        drawWall(j,    4, room[1][1]*room_size, room[1][0],  0,   1,   0, 0, 0, -1);
        drawWall(j,    8, room[1][1]*room_size, room[1][0],  0,   1,   0, 0, 0, -1);
    }

    // Draw left and right walls
    for (i = 2; i < 4; i++) {
        for (j=2; j <= 18; j+= 4) {
            drawWall(room[i][1]*10, 0, -room_size+j, room[i][0], 0, 1, 0, 0, 0, 1);
            drawWall(room[i][1]*10, 4, -room_size+j, room[i][0], 0, 1, 0, 0, 0, 1);
            drawWall(room[i][1]*10, 8, -room_size+j, room[i][0], 0, 1, 0, 0, 0, 1);
        }
    }
}

void drawR3Spots(double x, double y, double z) {
    //  Save transformation
    glPushMatrix();

    glTranslated(x, y, z);

    createSpotLight(0.0,   0.0,  15.0,      0.3,  1.0,  0.3,          -85.4, -94.0, -70.8,    20.8, 65.0, GL_LIGHT1);
    createSpotLight(0.0, -15.0, -15.0,      0.3,  1.0,  0.3,           -2.0, 321.2, -97.7,    25.0, 40.0, GL_LIGHT2);

    // back
    createSpotLight(7.0,  -7.0, 20.0,      0.0,  0.0,  1.0,           36.3, 17.7, -87.7,    20.0, 65.0, GL_LIGHT3);
    createSpotLight(-22.0,  -3.0, -28.0,   0.0,  0.0,  1.0,           374.5, -22.8, -147.9,    20.0, 36.2, GL_LIGHT4);

    // Lights over turrets
    createSpotLight(7.0,   0.0,  12.5,      1.0,  1.0,  1.0,          -1.9, -206.1, -7.2,    10.0, 92.4, GL_LIGHT5);
    createSpotLight(7.0,   0.0,  8.0,      1.0,  1.0,  1.0,          -1.9, -206.1, -7.2,    10.0, 92.4, GL_LIGHT6);
    createSpotLight(7.0,   0.0,  0.0,      1.0,  1.0,  1.0,          -1.9, -206.1, -7.2,    10.0, 92.4, GL_LIGHT7);


    glPopMatrix();
}

/**
 * Draw fog throughout scene
 */
void drawFog() {
    //  Save transformation
    glPushMatrix();
    glEnable (GL_DEPTH_TEST);

    GLfloat density = 0.01;
    GLfloat fogColor[4] = {0.5, 0.5, 0.5, 1.0};

    glEnable(GL_FOG);
    glFogi (GL_FOG_MODE, GL_EXP);

    glFogfv (GL_FOG_COLOR, fogColor);

    glFogf (GL_FOG_DENSITY, density);
    glHint (GL_FOG_HINT, GL_NICEST);

    glPopMatrix();
}

void drawRoom3(int room_size, int room[4][2]) {
    int j;

    drawR3Spots(43.0, 9.0, 0.0);
    

    int start = -2;
    int end = 50;
    // Draw Right Wall
    for (j = start; j <= end; j += 4) {
        drawWall(j, 0, room[0][1]*room_size, room[0][0], 0, 1, 0,     0, 0, -1);
        drawWall(j, 4, room[0][1]*room_size, room[0][0], 0, 1, 0,     0, 0, -1);
        drawWall(j, 8, room[0][1]*room_size, room[0][0], 0, 1, 0,     0, 0, -1);
    }

    float zn = -0.01314 * yrot - 1.1979;
    // Left Wall
    for (j = start+16; j <= end; j += 4) {
        drawWall(j, 0, room[1][1]*room_size-10, room[1][0], 0, 1, 0,     1.1, 0.0, zn);
        drawWall(j, 4, room[1][1]*room_size-10, room[1][0], 0, 1, 0,     1.1, 0.0, zn);
        drawWall(j, 8, room[1][1]*room_size-10, room[1][0], 0, 1, 0,     1.1, 0.0, zn);
    }
    
    // Far wall
    for (j=2; j <= 50; j+= 4) {
        drawWall(room_size*2+12, 0, room_size-j, room[2][0], 0, 1, 0,     -0.5, 0.0, -1.0);
        drawWall(room_size*2+12, 4, room_size-j, room[2][0], 0, 1, 0,     -0.5, 0.0, -1.0);
        drawWall(room_size*2+12, 8, room_size-j, room[2][0], 0, 1, 0,     -0.5, 0.0, -1.0);
    }

    // Corner wall
    for (j=2; j <= 10; j+= 4) {
        drawWall(-4, 0, room_size-j, room[2][0], 0, 1, 0, 0, 0, 1);
        drawWall(-4, 4, room_size-j, room[2][0], 0, 1, 0, 0, 0, 1);
        drawWall(-4, 8, room_size-j, room[2][0], 0, 1, 0, 0, 0, 1);
    }

    float i;
    float yn = 0.0209 * yrot + 1.862;
    // after pit
    for (i = end - 2; i < end + 3; i+=4) {
        for (j = -28; j < 21; j+=4) {
            drawWall(i, -2, j, 90, 90, 0, 0,     -1.0, yn, -1.0);
        }
    }
   
    // Draw Pit

    // Far Pit Wall
    for (j=2; j <= 50; j+= 4) {
        drawWall(room_size*2+6, -4, room_size-j, room[2][0], 0, 1, 0,      -0,  1, -1);
        drawWall(room_size*2+6, -8, room_size-j, room[2][0], 0, 1, 0,      -0,  1, -1);
        drawWall(room_size*2+6, -12, room_size-j, room[2][0], 0, 1, 0,     -0,  1, -1);
    }
   
    // Near Pit Wall 
    for (j=2; j <= 50; j+= 4) {
        drawWall(room_size*2-2, -4, room_size-j, room[2][0], 0, 1, 0,       -13.3, 3.6, 2.6);
        drawWall(room_size*2-2, -8, room_size-j, room[2][0], 0, 1, 0,       -13.3, 3.6, 2.6);
        drawWall(room_size*2-2, -12, room_size-j, room[2][0], 0, 1, 0,      -13.3, 3.6, 2.6);
    }

    // Right Pit Wall - Correct
    for (j = 40; j <= 44; j += 4) {
        drawWall(j, -4, room[0][1]*room_size, room[0][0], 0, 1, 0,           0, 0, -1);
        drawWall(j, -8, room[0][1]*room_size, room[0][0], 0, 1, 0,           0, 0, -1);
        drawWall(j, -10, room[0][1]*room_size, room[0][0], 0, 1, 0,          0, 0, -1);
    }
    
    // Left Pit Wall - Correct
    for (j = 40; j <= 44; j += 4) {
        drawWall(j, -4, room[1][1]*room_size-10, room[1][0], 0, 1, 0,        0, 0, -1);
        drawWall(j, -8, room[1][1]*room_size-10, room[1][0], 0, 1, 0,        0, 0, -1);
        drawWall(j, -10, room[1][1]*room_size-10, room[1][0], 0, 1, 0,       0, 0, -1);
    }

    // Pit Floor - Correct
    for (i = end - 12; i < end; i+=4) {
        for (j = -28; j < 21; j+=4) {
            drawWall(i, -12, j, 90, 90, 0, 0,       0,  1,  0);
        }
    }

    drawFog();

    companionCube(50, -1, -20, 1.0);
    companionCube(50, -1, -23, 1.0);
    companionCube(50, 0.95, -21.5, 1.0);

    drawTurret(50, 0, 0, 0);
    drawTurret(50, 0, 8, 0);
    drawTurret(50, 0, 12, 0);

    glColor4f(1.0f,1.0f,1.0f,0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);

    drawGlass(46, 0, 12, 90, 0, 90, 0, 8, 2, 0, 0, 0);
    drawGlass(46, 0, -22, 90, 0, 90, 0, 8, 2, 0, 0, 0);

    glDisable(GL_BLEND);
}
    
    
void drawRoom2(int room_size, int room[4][2]) {
    int i, j;
    double offset = -2;

    // Draw back wall
    for (j = -8; j <= 10; j += 4) {
        drawWall(j, 0, room[1][1]*room_size*4-offset, room[1][0], 0, 1, 0, 0, 0, -1);
        drawWall(j, 4, room[1][1]*room_size*4-offset, room[1][0], 0, 1, 0, 0, 0, -1);
        drawWall(j, 8, room[1][1]*room_size*4-offset, room[1][0], 0, 1, 0, 0, 0, -1);
    }

    // Draw front wall
    // Draw lower open wall
    drawWall(-8, 0, room[1][1]*room_size*2-offset, room[1][0],  0,   1,   0, 0, 0, -1);
    drawWall(-4, 0, room[1][1]*room_size*2-offset, room[1][0],  0,   1,   0, 0, 0, -1);
    drawWall(4, 0, room[1][1]*room_size*2-offset, room[1][0],  0,   1,   0, 0, 0, -1);
    drawWall(8, 0, room[1][1]*room_size*2-offset, room[1][0],  0,   1,   0, 0, 0, -1);

    for (j = -8; j <= 10; j += 4) {
        drawWall(j,    4, room[1][1]*room_size*2-offset, room[1][0],  0,   1,   0, 0, 0, -1);
        drawWall(j,    8, room[1][1]*room_size*2-offset, room[1][0],  0,   1,   0, 0, 0, -1);
    }

    // Draw left and right walls
    for (i = 2; i < 4; i++) {
        for (j=2; j <= 18; j+= 4) {
            drawWall(room[i][1]*10, 0, -(room_size*4)+j-offset, room[i][0], 0, 1, 0, 0, 0, 1);
            drawWall(room[i][1]*10, 4, -(room_size*4)+j-offset, room[i][0], 0, 1, 0, 0, 0, 1);
            drawWall(room[i][1]*10, 8, -(room_size*4)+j-offset, room[i][0], 0, 1, 0, 0, 0, 1);
        }
    }

    // Far upper ceiling
    for (i = 0; i < 20; i+=4) {
        drawWall(-8+i, 8, -34, 0, 0, 1, 0, 0, 0, 1);
        drawWall(-8+i, 6, -36, 90, 90, 0, 0, 0, 0, 1);
    }
   
    // Draw right bulging wall 
    drawWall(8, 0, -30, 90, 0, 1, 0, 0, 0, -1);
    drawWall(8, 0, -26, 90, 0, 1, 0, 0, 0, -1);
    drawWall(10, 0, -24, 0, 0, 1, 0, 0, 0, -1);
    drawWall(10, 0, -32, 0, 0, 1, 0, 0, 0, -1);
}

void drawHall(int room_size, int room[4][2]) {
    int i, j;

    for (i = 2; i < 4; i++) {
        for (j=2; j <= 8; j+= 4) {
            drawWall(room[i][1]*2.0, 0, -room_size-j, room[i][0], 0, 1, 0, 0, 0, 1);
        }
    }
   
    // Draw Ceiling 
    drawWall(0.0, 2, -12, 90, 90, 0, 0, 0, 0, -1);
    drawWall(0.0, 2, -16, 90, 90, 0, 0, 0, 0, -1);
}

/*
 * Draws the scene.
 */
void drawScene() {
    int room_size = 10;

    camera(); 

    int room[4][2] = {
        {  0,  1}, // front
        {180, -1}, // back
        { 90, -1}, // left
        {-90,  1}  // right
    };


    // Draw floor    
    float i,j;
    for (i = -8; i < 37; i+=4) {
        for (j = -36; j < 21; j+=4) {
            drawWall(i, -2, j, 90, 90, 0, 0, 0, 0, -1);
        }
    }
    for (i = -8; i < 54; i+=4) {
        for (j = -36; j < 24; j+=4) {
            drawWall(i, 10, j, 90, 90, 0, 0, 0, 0, 1);
        }
    }

    drawRoom1(room_size, room);
    drawRoom2(room_size, room);
    drawHall(room_size, room);
    drawRoom3(room_size*2, room);


    companionCube(8.0, -1, -35, 1.0);

    glDisable(GL_TEXTURE_2D);

}

/*
 * Sets up and draws the scene.
 */
void display() {
    // Clear Color and Depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Enable Z-Buffering 
    glEnable(GL_DEPTH_TEST);
    // Undo previous transformations
    glLoadIdentity();
    // Set view mode
    glShadeModel(GL_SMOOTH);

    //------------------------------------------
    //  Enable lighting
    glEnable(GL_LIGHTING);

    drawScene();

    // Global light
    //            x     y    z   ambient    diffuse    specular   light_en
    createLight(0.0,  15.0, 0.0,   0.0,       op,        0.0,    GL_LIGHT0);

    // Disable Lighting
    glDisable(GL_LIGHTING);
    //------------------------------------------

    //  White
    glColor3f(1,1,1);
    //  Display parameters
    glWindowPos2i(5,25);
    Print("xpos=%.1f, zpos=%.1f, yrot=%.1f", xpos, zpos, yrot);

    //  Render the scene and make it visible
    ErrCheck("display");
    //  Render the scene
    glFlush();
    //  Make the rendered scene visible
    glutSwapBuffers();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
    //  Ratio of the width to the height of the window
    asp = (height>0) ? (double)width/height : 1;
    //  Set the viewport to the entire window
    glViewport(0,0, width,height);
    //  Set projection
    Project(fov,asp,dim);
}

/*
 * The idle method the animcate the light source movement.
 */
void idle() {
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/* 
 * Handles keyboard events.
 */
void key(unsigned char ch,int x,int y)
{
    //  Exit on ESC
    if (ch == 27)
       exit(0);

    // Update projection
    Project(fov,asp,dim);

    //  Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}

//void key(unsigned char key,int x,int y)
void special(int key,int x,int y)
{
    /* avoid thrashing this procedure */
    usleep(100);

    switch (key) {
        case GLUT_KEY_PAGE_UP: // tilt up
            z -= 0.2f;
            lookupdown -= 2.0f;
            break;

        case GLUT_KEY_PAGE_DOWN: // tilt down
            z += 0.2f;
            lookupdown += 2.0f;
            break;

        case GLUT_KEY_UP: // walk forward (bob head)
            xpos -= (float)sin(yrot*radians) * 0.35f;
            zpos -= (float)cos(yrot*radians) * 0.35f;
            break;

        case GLUT_KEY_DOWN: // walk back (bob head)
            xpos += (float)sin(yrot*radians) * 0.35f;
            zpos += (float)cos(yrot*radians) * 0.35f;
            break;

        case GLUT_KEY_LEFT: // look left
            yrot += 3.5f;
            break;

        case GLUT_KEY_RIGHT: // look right
            yrot -= 3.5f;
            break;

        default:
            printf ("Special key %d pressed. No action there yet.\n", key);
            break;
    }
}

/*
 * Handles initialization for the scene setup.
 */
void init() {
    texture[0] = LoadTexBMP("concrete.bmp");
    texture[1] = LoadTexBMP("companion_cube.bmp");
    texture[2] = LoadTexBMP("glass.bmp");
    texture[3] = LoadTexBMP("turret.bmp");
}

int main(int argc, char * argv[])
{

    // Initialize GLUT and process user params
    glutInit(&argc,argv);
    
    // Request double buffered, true color window
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    
    // Set the window size to 500 x 500 pixels
    glutInitWindowSize(600, 600);
    
    // Create window with assignment name in title
    glutCreateWindow("My Portal");
    
    // Register the display method with GLUT
    glutDisplayFunc(display);
    
    // Tell GLUT which method to use when the window is resized
    glutReshapeFunc(reshape);

    // Tell GLUT which method to use when at idle.
    glutIdleFunc(idle);

    // Tell GLUT which method to use for special keys.
    glutSpecialFunc(special);

    // Tell GLUT which method to use for keyboard events.
    glutKeyboardFunc(key);

    // Initialize components for the scene.
    init();

    ErrCheck("init");
    
    // Start the main loop.
    glutMainLoop();
    
    return 0;
}

