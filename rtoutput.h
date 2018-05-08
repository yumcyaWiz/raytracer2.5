#ifndef RTOUTPUT_H
#define RTOUTPUT_H
#include <GL/glut.h>
#include <omp.h>
#include "integrator.h"


//RealTime output
Integrator* integ_rtoutput;
Scene scene_rtoutput;

static int n_rtoutput = 0;
void idle(void) {
    integ_rtoutput->compute(scene_rtoutput);
    n_rtoutput++;
    glutPostRedisplay();
}
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_POINTS);
    int width = integ_rtoutput->cam->film->width;
    int height = integ_rtoutput->cam->film->height;
    for(int i = 0; i < width; i++) {
        for(int j = 0; j < height; j++) {
            RGB c = integ_rtoutput->cam->film->getPixel(i, j)/n_rtoutput;
            c.x = std::pow(c.x, 1.0f/2.2f);
            c.y = std::pow(c.y, 1.0f/2.2f);
            c.z = std::pow(c.z, 1.0f/2.2f);
            c = max(c, RGB(0.0f));
            c = min(c, RGB(1.0f));
            glColor3d(c.x, c.y, c.z);
            glVertex2d((2.0*i - width)/width, 1.0f - (2.0f*j - height)/height - 1.0f);
        }
    }
    glEnd();
    glutSwapBuffers();
}


void resize(int w, int h) {
}


void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
}


void drawgl(int argc, char** argv, Integrator* integ, const Scene& scene) {
    integ_rtoutput = integ;
    scene_rtoutput = scene;
    glutInitWindowSize(integ->cam->film->width, integ->cam->film->height);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA);
    glutCreateWindow(argv[0]);
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutIdleFunc(idle);
    init();
    glutMainLoop();
}
#endif
