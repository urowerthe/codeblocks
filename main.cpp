#include <GL/glut.h>
#include <iostream>
#include <math.h>
#include <cstdlib> // Para rand() y srand()
#include <ctime>   // Para time()

using namespace std;

float camaraX = -20;
float camaraY = 15;
float camaraZ = 30;
float angulo = -16.7;
float radio = 30;
float centroY = 10;
float posLuzX = 0;
float posLuzY = 40;
float posLuzZ = 30;

// Quadric para cilindros/discos
GLUquadricObj* quadric;

float rioOffset = 0.0f;

// --- (NUEVO) Variables de estado del Personaje ---
float personajeX = 0.0f;       // Posición X del personaje
float personajeZ = 20.0f;       // Posición Z del personaje
float personajeAngulo = M_PI;   // Dirección a la que mira el personaje (en radianes)
float velocidad = 0.05f;         // Velocidad de movimiento del personaje

// --- (NUEVO) Variables para la animación ---
float faseCaminata = 0.0f;      // Controla la animación de las piernas al caminar
bool pateando = false;          // Estado: ¿está pateando actualmente?
float fasePateo = 0.0f;         // Controla la animación de la patada

// NUEVAS VARIABLES PARA LA PAUSA ANTES DE PATEAR
bool enPausaParaPatear = false; // true cuando el personaje está en la pausa
int contadorPausa = 0;
// --- Variables para el movimiento automático ---
float puntoInicioZ = 20.0f;
float puntoFinalZ = -25.0f;
bool haciaElFinal = true;

// --- (MODIFICADO) Variables del Cubo con Física ---
float tractorX = -1.0f;
float tractorY = 1.0f;
float tractorZ = 0.0f;
float tractorVelocidadX = 0.0f; // NUEVO: Velocidad del cubo en el eje X
float tractorVelocidadZ = 0.0f; // NUEVO: Velocidad del cubo en el eje Z
bool impulsoAplicado = false; // NUEVO: Para asegurar que la patada da un solo impulso

// Variable global para la inclinación de la cabeza (agrega cerca de las otras variables globales)
float inclinacionCabeza = 0.0f;

float estirarBrazoDerecho = 0.0f; // 0: normal, 1: estirado

float puertaAngulo = 0.0f;           // Ángulo de apertura de la puerta (0=cerrada, 90=abierta)
bool abriendoPuerta = false;         // Estado: animando apertura
bool cerrandoPuerta = false;         // Estado: animando cierre
bool robotEntrando = false;          // Estado: robot entrando a la casa
int contadorEsperaPuerta = 0;        // Para pausar antes de cerrar la puerta

//movimiento de camara

float camX = 0.0f, camY = 5.0f, camZ = 20.0f;     // Posición de la cámara
float dirX = 0.0f, dirY = 0.0f, dirZ = -1.0f;     // Dirección de la cámara
float anguloCamara = 0.0f;                        // Ángulo de rotación en Y (horizontal)
float velocidadCamara = 0.5f;                     // Velocidad de movimiento


void iniciarVentana(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(80.0, (float)w / (float)h, 1, 200);
    rioOffset += 0.01f;
    if (rioOffset > 2.0f) rioOffset = 0.0f;  // reinicio para evitar overflow
    glutPostRedisplay();
}
void inicializarLuces() {
    GLfloat light_ambient[] = { 0.3,0.3,0.3,1 };
	GLfloat light_diffuse[] = { 0.8,0.8,0.8,1 };
	GLfloat light_specular[] = { 0.4,0.4,0.4,1 };

	float reflejo[] = { 0,0,0,1 };
	float posicionLuz[] = { posLuzX,posLuzY,posLuzZ,1 };
	int sombra = 128;

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT, GL_SPECULAR, reflejo);
	glMateriali(GL_FRONT, GL_SHININESS, sombra);
	glLightfv(GL_LIGHT0, GL_POSITION, posicionLuz);
}

void dibujarEjes(){
    glPushMatrix();
        glBegin(GL_LINES);
            glColor3ub(255,0,0);
            glVertex3d(0,0,0);
            glVertex3d(50,0,0);

            glColor3ub(0,255,0);
            glVertex3d(0,0,0);
            glVertex3d(0,50,0);

            glColor3ub(0,0,255);
            glVertex3d(0,0,0);
            glVertex3d(0,0,50);
        glEnd();
    glPopMatrix();
}

void cieloRasoColor() {
    glDisable(GL_LIGHTING);
    glColor3ub(135, 206, 235); // Celeste claro tipo cielo

    glPushMatrix();
        glTranslated(0, 60, 0);
        glBegin(GL_QUADS);
            glVertex3f(-50, 0, -50);
            glVertex3f( 50, 0, -50);
            glVertex3f( 50, 0,  50);
            glVertex3f(-50, 0,  50);
        glEnd();
    glPopMatrix();

    glEnable(GL_LIGHTING);
}

void arbolNavidad(float x, float z) {
    glPushMatrix();
    glTranslated(x, 0, z);

    float escala = 1.8;  // 🔺 Factor de escala para hacerlo más grande

    // Tronco más alto y grueso
    glColor3ub(101, 67, 33);
    glPushMatrix();
        glTranslated(0, escala * 0.1, 0);
        glRotated(-90, 1, 0, 0);
        gluCylinder(gluNewQuadric(), 0.5 * escala, 0.5 * escala, 3 * escala, 20, 20);
    glPopMatrix();

    // Conos verdes apilados
    glColor3ub(0, 100, 0);
    for (int i = 0; i < 3; ++i) {
        glPushMatrix();
            glTranslated(0, escala * (3.0 + i * 2.0), 0);
            glRotated(-90, 1, 0, 0);
            glutSolidCone((3.0 - i * 0.6) * escala, 2.0 * escala, 20, 20);
        glPopMatrix();
    }

    glPopMatrix();
}


void dibujarArbolesNavidad() {
    arbolNavidad(-20, -10);
    arbolNavidad(15, -25);
    arbolNavidad(-25, 15);
    arbolNavidad(20, 20);
}

void pasto() {
    glColor3f(0.0f, 0.8f, 0.0f);
    glTranslatef(0, -0.45, 0);
    glBegin(GL_QUADS);
      glVertex3d(-40, 0.5, -30);
      glVertex3d(40, 0.5, -30);
      glVertex3d(40, 0.5, 30);
      glVertex3d(-40, 0.5, 30);
    glEnd();
}


void colinaRealista(float x, float y, float z, float radio, float altura) {
    glPushMatrix();
    glTranslated(x, y, z);
    glScalef(1.0, altura / radio, 1.0); // Aplana verticalmente
    glColor3f(0.1f, 0.5f, 0.1f);
    glutSolidSphere(radio, 50, 50);  // Más segmentos para suavidad
    glPopMatrix();
}


void paisaje() {
     // Colinas al fondo
    colinaRealista(-25, 0.5f, -35, 10.0f, 5.0f);
    colinaRealista(-10, 0.5f, -30, 8.0f, 4.0f);
    colinaRealista(10, 0.5f, -32, 9.0f, 4.5f);
    colinaRealista(25, 0.5f, -34, 11.0f, 5.5f);
}

// Dibuja un tronco (cilindro horizontal)
void dibujarTronco(float x, float y, float z, float largo, float radio) {
    const int slices = 20;
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(90, 0, 1, 0);
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, radio, radio, largo, slices, 1);
    gluDisk(quad, 0.0, radio, slices, 1);
    glTranslatef(0.0, 0.0, largo);
    gluDisk(quad, 0.0, radio, slices, 1);
    gluDeleteQuadric(quad);
    glPopMatrix();
}

// Dibuja una pared de troncos apilados
void paredTroncos(float largo, float alto, float prof, float radio) {
    int cantidad = alto / (2 * radio);
    for (int i = 0; i < cantidad; ++i) {
        dibujarTronco(0, i * 2 * radio, 0, largo, radio);
    }
}

// Dibuja la casa rústica
void dibujarCasaRustica() {
    float radioTronco = 0.4f;
    float altoPared = 7.0f;
    float largoCasa = 11.0f;
    float anchoCasa = 3.9f;

    // Pared frontal
    glPushMatrix();
    glTranslatef(-largoCasa / 2, 0, -anchoCasa / 0.21);
    glColor3ub(184, 134, 11);
    paredTroncos(largoCasa, altoPared, 1, radioTronco);
    glPopMatrix();

    // Pared posterior
    glPushMatrix();
    glTranslatef(-largoCasa / 2, 0, anchoCasa / -0.17);
    glColor3ub(184, 134, 11);
    paredTroncos(largoCasa, altoPared, 1, radioTronco);
    glPopMatrix();

    // Pared izquierda
    glPushMatrix();
    glTranslatef(-largoCasa / 2, 0, -anchoCasa / 0.21);
    glRotatef(90, 0, 2, 0);
    paredTroncos(anchoCasa, altoPared, 1, radioTronco);
    glPopMatrix();

    // Pared derecha
    glPushMatrix();
    glTranslatef(largoCasa / 2, 0, -anchoCasa / 0.21);
    glRotatef(90, 0, 1, 0);
    paredTroncos(anchoCasa, altoPared, 1, radioTronco);
    glPopMatrix();

    // Techo (dos triángulos planos)
    glPushMatrix();
    glTranslatef(0, altoPared -1.1f, -20.66);
    glColor3ub(139, 69, 19);
    glBegin(GL_TRIANGLES);
    glVertex3f(-largoCasa /2, 0, -anchoCasa / 2);
    glVertex3f(largoCasa / 2, 0, -anchoCasa / 2);
    glVertex3f(0,1.5f, -anchoCasa / 4);
    glVertex3f(-largoCasa / 2, 0, anchoCasa / 2);
    glVertex3f(largoCasa / 2, 0, anchoCasa / 2);
    glVertex3f(0, 1.5f, anchoCasa / 4);

    glEnd();

    // Plano del techo
    glBegin(GL_QUADS);
    glVertex3f(-largoCasa / 2, 0, -anchoCasa / 2);
    glVertex3f(-largoCasa / 2, 0, anchoCasa / 2);
    glVertex3f(0, 1.5f, anchoCasa / 2);
    glVertex3f(0, 1.5f, -anchoCasa / 2);


    glVertex3f(largoCasa / 2, 0, -anchoCasa / 2);
    glVertex3f(largoCasa / 2, 0, anchoCasa / 2);
    glVertex3f(0, 1.5f, anchoCasa / 2);
    glVertex3f(0, 1.5f, -anchoCasa / 2);

    glEnd();
    glPopMatrix();

    // Chimenea
    glPushMatrix();
    glTranslatef(-2.9, altoPared + 0.1f, -20.9f);
    glColor3ub(120, 120, 120);
    glScalef(0.5, 4.0, 0.5);
    glutSolidCube(1.5);
    glPopMatrix();

    // PUERTA (frontal)
    glPushMatrix();
        glColor3ub(0, 0, 0); // Marrón oscuro
        glTranslatef(-0.6f, 1.0f, -18.2f); // Centrada y al frente
        glScalef(1.2f, 4.0f, 0.1f);
        glutSolidCube(1.5f);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(0.6f, 1.0f, -18.2f); // Ajusta X para el eje de giro (bisel)
        glRotatef(puertaAngulo, 0, 1, 0); // Aplica la rotación de apertura
        glTranslatef(-0.6f, 0.0f, 0.0f); // Regresa el centro de la puerta
        glColor3ub(101, 67, 33); // Marrón oscuro
        glScalef(1.2f, 4.0f, 0.1f);
        glutSolidCube(1.5f);
    glPopMatrix();



    // VENTANA LATERAL IZQUIERDA
    glPushMatrix();
    glTranslatef(-3.01f, 2.0f, -18.2f); // izquierda
    glColor3ub(173, 216, 230); // Azul claro (vidrio)
    glScalef(1.2f, 1.2f, 0.1f);
    glutSolidCube(1.5f);
    glPopMatrix();

    // VENTANA LATERAL DERECHA
    glPushMatrix();
    glTranslatef(3.01f, 2.0f, -18.2f); // derecha
    glColor3ub(173, 216, 230);
    glScalef(1.2f, 1.2f, 0.1f);
    glutSolidCube(1.5f);
    glPopMatrix();

    // ESCALERA DE PIEDRA
    glColor3ub(130, 130, 130); // gris piedra
    for (int i = 0; i < 3; ++i) {
        glPushMatrix();
        glTranslatef(0.0f, 0.2f * i, -18.0f - i * 0.5f);
        glScalef(2.0f - i * 0.2f, 0.2f, 1.0f);
        glutSolidCube(1.5f);
        glPopMatrix();
    }
}

void dibujarNube(float x, float y, float z) {
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glColor3ub(255, 255, 255);
    glTranslatef(x, y, z);
    glutSolidSphere(4, 20, 20);
    glTranslatef(5, 0, 1);
    glutSolidSphere(3, 20, 20);
    glTranslatef(-10, 0, -2);
    glutSolidSphere(3.5, 20, 20);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void dibujarBanca() {
    glPushMatrix();
    glColor3ub(128, 82, 45);
    glPushMatrix();
    glTranslatef(0, 1.5, 0);
    glScalef(6.0, 0.2, 1.0);
    glutSolidCube(1.0);
    glPopMatrix();

    glColor3ub(128, 128, 128);
    glPushMatrix();
    glTranslatef(-2.5, 0.75, 0);
    glScalef(0.5, 1.5, 0.8);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(2.5, 0.75, 0);
    glScalef(0.5, 1.5, 0.8);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
}

// Funciones de dibujo del ambiente (sin cambios)
void dibujarCielo() {
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glColor3ub(135, 206, 235);
    glutSolidSphere(90, 30, 30);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void sendero() {
    // Camino de tierra
    glColor3f(0.5f, 0.35f, 0.05f);  // color marrón claro
    glBegin(GL_QUADS);
    glVertex3d(-1.5f, 0.51f, -18.0f);
    glVertex3d(1.5f, 0.51f, -18.0f);
    glVertex3d(3.0f, 0.51f, 30.0f);
    glVertex3d(-3.0f, 0.51f, 30.0f);
    glEnd();

    // Piedritas decorativas en los bordes
    glColor3f(0.4f, 0.4f, 0.4f);
    for (float i = 10.0f; i <= 30.0f; i += 2.0f) {
        glPushMatrix();
        glTranslatef(-2.0f, 0.52f, i);
        glutSolidSphere(0.15f, 8, 8);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(2.0f, 0.52f, i);
        glutSolidSphere(0.15f, 8, 8);
        glPopMatrix();
    }
}

//PIES
void pie(float x) {
    glColor3ub(150, 75, 0);
    GLdouble plano[] = {0.0, 1.0, 0.0, 0.0};
    glEnable(GL_CLIP_PLANE0);
    glClipPlane(GL_CLIP_PLANE0, plano);
    glPushMatrix();
    glTranslatef(x, 0, 0);
    glRotatef(180, 1, 0, 0); // Boca abajo
    glutSolidSphere(2.0, 30, 30);
    glPopMatrix();

    glDisable(GL_CLIP_PLANE0);
}

//PIERNAS
void pierna(float x,float angle) {
    //pierna izquierda
    glPushMatrix();
    glColor3ub(100, 100, 100);
    glTranslatef(0,7,0);
    glRotatef(-angle, 1, 0, 0);
    glTranslatef(0,-7,0);
    for (int i = 0; i < 5; i++) {
       glPushMatrix();
        glTranslatef(x, 1.8 + i * 1.2, 0);
        glRotatef(-90, 1, 0, 0);
        glScaled(1.5,1.5,1.5);
        gluCylinder(gluNewQuadric(), 0.4, 0.4, 1.2, 20, 1);
        glPopMatrix();
    }

    pie(x);

    glPopMatrix();
        //pierna derecha
    glPushMatrix();
    glColor3ub(100, 100, 100);
    glTranslatef(0,7,0);
    glRotatef(angle, 1, 0, 0);
    glTranslatef(0,-7,0);
    for (int i = 0; i < 5; i++) {
       glPushMatrix();
        glTranslatef(-x, 1.8 + i * 1.2, 0);
        glRotatef(-90, 1, 0, 0);
        glScaled(1.5,1.5,1.5);
        gluCylinder(gluNewQuadric(), 0.4, 0.4, 1.2, 20, 1);
        glPopMatrix();
    }
    pie(-x);

    glPopMatrix();
}

void piernaPateo(float x, float anguloPateo) {
    //pierna izquierda
    glPushMatrix();
    glColor3ub(100, 100, 100);
    glTranslatef(0,7,0);
    glRotatef(anguloPateo, 1, 0, 0);
    glTranslatef(0,-7,0);
    for (int i = 0; i < 5; i++) {
       glPushMatrix();
        glTranslatef(x, 1.8 + i * 1.2, 0);
        glRotatef(-90, 1, 0, 0);
        glScaled(1.5,1.5,1.5);
        gluCylinder(gluNewQuadric(), 0.4, 0.4, 1.2, 20, 1);
        glPopMatrix();
    }

    pie(x);

    glPopMatrix();
        //pierna derecha
    glPushMatrix();
    glColor3ub(100, 100, 100);
    glTranslatef(0,7,0);
    glRotatef(-5, 1, 0, 0);
    glTranslatef(0,-7,0);
    for (int i = 0; i < 5; i++) {
       glPushMatrix();
        glTranslatef(-x, 1.8 + i * 1.2, 0);
        glRotatef(-90, 1, 0, 0);
        glScaled(1.5,1.5,1.5);
        gluCylinder(gluNewQuadric(), 0.4, 0.4, 1.2, 20, 1);
        glPopMatrix();
    }
    pie(-x);

    glPopMatrix();
}

//CINTURA
void cintura() {
    glPushMatrix();
    glTranslatef(0, 8.5, 0);
    glScalef(9, 2, 3);
    glutSolidCube(1.0);
    glPopMatrix();
}

//TORSO
void torso() {
    glPushMatrix();
    glTranslated(0,8.9,0);
    glScaled(2.7,2.7,2.7);
    // Coordenadas de la base inferior (más ancha)
    float b = 2.0; // semiancho inferior
    // Coordenadas de la base superior (más estrecha)
    float t = 1.0; // semiancho superior
    float h = 2.5; // altura
    float d = 1.0; // profundidad

    // Cara frontal
    glBegin(GL_QUADS);
        glVertex3f(-b, 0.0, d);  // abajo izquierda
        glVertex3f(b, 0.0, d);   // abajo derecha
        glVertex3f(t, h, d);     // arriba derecha
        glVertex3f(-t, h, d);    // arriba izquierda
    glEnd();

    // Cara trasera
    glBegin(GL_QUADS);
        glVertex3f(-b, 0.0, -d);
        glVertex3f(b, 0.0, -d);
        glVertex3f(t, h, -d);
        glVertex3f(-t, h, -d);
    glEnd();

    // Lado izquierdo
    glBegin(GL_QUADS);
        glVertex3f(-b, 0.0, -d);
        glVertex3f(-b, 0.0, d);
        glVertex3f(-t, h, d);
        glVertex3f(-t, h, -d);
    glEnd();

    // Lado derecho
    glBegin(GL_QUADS);
        glVertex3f(b, 0.0, -d);
        glVertex3f(b, 0.0, d);
        glVertex3f(t, h, d);
        glVertex3f(t, h, -d);
    glEnd();

    // Cara superior
    glBegin(GL_QUADS);
        glVertex3f(-t, h, -d);
        glVertex3f(t, h, -d);
        glVertex3f(t, h, d);
        glVertex3f(-t, h, d);
    glEnd();

    // Cara inferior
    glBegin(GL_QUADS);
        glVertex3f(-b, 0.0, -d);
        glVertex3f(b, 0.0, -d);
        glVertex3f(b, 0.0, d);
        glVertex3f(-b, 0.0, d);
    glEnd();
    glPopMatrix();

    // Pantalla (cuadro plano)
    glPushMatrix();
    glTranslatef(0, 13.2, 2.7);
    glScalef(4, 2, 0.1);
    glutSolidCube(1.0);
    glPopMatrix();

    // Botones (3 discos planos)
    for (float x = -2.8; x <= 2.8; x += 2.8) {
        glPushMatrix();
        glColor3ub(255, 0, 0);
        glTranslatef(x, 10.5, 2.8);
        glScalef(1.7, 1.7, 1.7);
        gluDisk(gluNewQuadric(), 0.0, 0.5, 20, 1);
        glPopMatrix();
    }
}


void cuello() {
    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glColor3ub(255, 0, 0);
        glTranslated(0, 15.6 + i * 0.3, 0);
        glScalef(3, 0.5, 3);
        glRotatef(-90, 1, 0, 0);
        gluCylinder(gluNewQuadric(), 0.5, 0.5, 0.6, 20, 1);
        glPopMatrix();
    }
}

//CABEZA
void Cabeza() {
    glPushMatrix();
    glTranslated(0, 7.5, 0);

    // INCLINACIÓN DE LA CABEZA
    glTranslatef(0, 12, 0); // Lleva el pivote a la base de la cabeza
    glRotatef(+inclinacionCabeza, 1, 0, 0); // Aplica la inclinación
    glTranslatef(0, -12, 0); // Regresa el pivote

    // Cubo de la cabeza
    glColor3ub(200, 200, 200);
    glPushMatrix();
        glTranslatef(0, 12, 0);
        glScalef(8, 6, 8);
        glutSolidCube(1);
    glPopMatrix();

    // Ojos nivel 1
    for (int i = -1; i <= 1; i += 2) {
        glPushMatrix();
        glColor3ub(170, 170, 170);
        glTranslated(i * 2.0, 13.0, 4.0);
        gluCylinder(quadric, 1.2, 1.2, 0.4, 50, 1);
        gluDisk(quadric, 0, 1.2, 50, 1);
        glPushMatrix();
            glTranslated(0, 0, 0.4);
            gluDisk(quadric, 0, 1.2, 50, 1);
        glPopMatrix();
        glPopMatrix();
    }

    // Ojos nivel 2
    for (int i = -1; i <= 1; i += 2) {
        glPushMatrix();
        glColor3ub(255, 255, 255);
        glTranslated(i * 2.0, 13.0, 4.3);
        gluCylinder(quadric, 0.8, 0.8, 0.2, 50, 1);
        gluDisk(quadric, 0, 0.8, 50, 1);
        glPushMatrix();
            glTranslated(0, 0, 0.2);
            gluDisk(quadric, 0, 0.8, 50, 1);
        glPopMatrix();
        glPopMatrix();
    }

    // Ojos nivel 3 (pupilas)
    glColor3ub(0, 0, 0);
    for (int i = -1; i <= 1; i += 2) {
        glPushMatrix();
        glTranslated(i * 2.0, 13.0, 4.2);
        glutSolidSphere(0.6, 12, 12);
        glPopMatrix();
    }

    // Reflejo ojo
    glColor3ub(255, 255, 255);
    for (int i = -1; i <= 1; i += 2) {
        glPushMatrix();
        glTranslated(i * 2.1, 13.1, 4.5);
        glutSolidSphere(0.3, 12, 12);
        glPopMatrix();
    }

    // Orejas
    glColor3ub(170, 170, 170);
    for (int i = -1; i <= 1; i += 2) {
        glPushMatrix();
        glTranslated(i * 4.5, 12.0, 0);
        glScaled(0.5, 1, 1);
        glutSolidCube(3.5);
        glPopMatrix();
    }
    glColor3ub(170, 170, 170);
    for (int i = -1; i <= 1; i += 2) {
        glPushMatrix();
        glTranslated(i * 5.0, 12.0, 0);
        glutSolidSphere(1.5, 12, 12);
        glPopMatrix();
    }

    // Boca (sonrisa)
    glPushMatrix();
    const float radio_interior = 1.5f;
    const float radio_exterior = 2.1f;
    const int num_dientes = 7;
    const float angulo_total = 90.0f;
    const float angulo_inicial = -angulo_total / 2.0f;
    glTranslated(0.0, 12, 4.1);
    glColor3ub(240, 240, 240);
    glRotated(-90.0, 0.0, 0.0, 1.0);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= num_dientes; ++i) {
        float angulo = angulo_inicial + (i * angulo_total / num_dientes);
        float angulo_rad = angulo * M_PI / 180.0;
        float x_ext = radio_exterior * cos(angulo_rad);
        float y_ext = radio_exterior * sin(angulo_rad);
        glNormal3f(0.0, 0.0, 1.0);
        glVertex3f(x_ext, y_ext, 0.0);
        float x_int = radio_interior * cos(angulo_rad);
        float y_int = radio_interior * sin(angulo_rad);
        glNormal3f(0.0, 0.0, 1.0);
        glVertex3f(x_int, y_int, 0.0);
    }
    glEnd();
    glColor3ub(10, 10, 10);
    glLineWidth(2.0);
    glTranslatef(0.0, 0.0, 0.01);
    glBegin(GL_LINES);
    for (int i = 0; i <= num_dientes; ++i) {
        float angulo = angulo_inicial + (i * angulo_total / num_dientes);
        float angulo_rad = angulo * M_PI / 180.0;
        float x_ext = radio_exterior * cos(angulo_rad);
        float y_ext = radio_exterior * sin(angulo_rad);
        float x_int = radio_interior * cos(angulo_rad);
        float y_int = radio_interior * sin(angulo_rad);
        glVertex3f(x_ext, y_ext, 0.0);
        glVertex3f(x_int, y_int, 0.0);
    }
    glEnd();
    glPopMatrix();

    // Antena (base + cilindro + esfera)
    glColor3ub(200, 200, 200);
    glPushMatrix();
        glTranslatef(0, 16, 0);
        glScalef(1, 1, 1);
        glutSolidCube(1);
    glPopMatrix();
    glColor3ub(180, 180, 180);
    glPushMatrix();
        glTranslated(0, 16.0, 0);
        glRotated(-90, 1, 0, 0);
        gluCylinder(quadric, 0.2f, 0.2f, 3.0f, 12, 3);
        glTranslated(0, 0, 3.0);
        glutSolidSphere(0.5f, 12, 12);
    glPopMatrix();

    glPopMatrix();
}
//fin de funcion

//MANOS
void Manos(float posX ,float direction) {
    glPushMatrix();
    glTranslated(posX,7.5,0);
    glRotated(90, 0.0, direction ,0.0);
    glRotated(-90, 1.0, 0.0, 0.0);
    glColor3ub(128, 128, 128);

    const int segments = 10;           // Cantidad de bloques para formar la curva
    const float segment_length = 0.5f;// Largo de cada bloque individual
    const float hand_width = 2.0f;    // Ancho total de la mano
    const float hand_thickness = 0.8f;// Grosor de la mano
    const float curve_angle = 19.0;   // Grados que se dobla cada segmento

    // Formar arco
    for (int i = 0; i < segments; i++) {
        // Dibuja el segmento actual
        glPushMatrix();
        glScalef(hand_width, hand_thickness, segment_length);
        glutSolidCube(1.0);
        glPopMatrix();
        // Mueve el sistema de coordenadas al final del segmento recién dibujado
        // y lo rota para preparar el siguiente. Así se forma la curva.
        glTranslatef(0.0, 0.0, segment_length);
        glRotated(curve_angle, 1.0, 0.0, 0.0); // Rota sobre X para curvar hacia abajo
    }

    glPopMatrix();
}

//BRAZOS
void Brazos(float angle, float estirarDerecho) {
      glColor3ub(100, 100, 100);

    const int   N        = 40;    // número de segmentos por brazo
    const float r        = 5.0f;  // radio del arco
    const float zArm     = 0;  // profundidad sobre el plano Z
    const float armRad   = 0.5f;  // radio del brazo (grosor)
    // --- Brazo izquierdo: de π/2 a 0 ---
    glPushMatrix();
    glTranslatef(0,14,0);
    //glRotatef(angle, 1, 0, 0);
    glRotatef(angle + estirarDerecho * 80.0f, 1, 0, 0); // +80 grados si estirado
    glTranslatef(0,-14,0);
    {
        float cx = 3.0f, cy = 9.0f;
        for(int i = 0; i < N; ++i) {
            float t0 = float(i)   / float(N);
            float t1 = float(i+1) / float(N);
            float a0 = M_PI/2 * (1.0f - t0);
            float a1 = M_PI/2 * (1.0f - t1);

            float x0 = cx + r * cosf(a0), y0 = cy + r * sinf(a0);
            float x1 = cx + r * cosf(a1), y1 = cy + r * sinf(a1);

            float dx = x1 - x0, dy = y1 - y0;
            float segLen = sqrtf(dx*dx + dy*dy);
            float theta  = atan2f(dy, dx) * 180.0f / M_PI;

            glPushMatrix();
                glTranslatef(x0, y0, zArm);
                glRotatef(theta, 0,0,1);       // alinea el eje X local con la tangente
                glRotatef(-90.0f, 0,1,0);      // rota para que el eje Z local (cylinder) apunte a +X
                gluCylinder(quadric, armRad, armRad, segLen, 8, 1);
            glPopMatrix();
        }
    }
    Manos(9.3,-1);
    glPopMatrix();

    // --- Brazo derecho: de π/2 a π ---
    glPushMatrix();
    glTranslatef(0,14,0);
    glRotatef(-angle, 1, 0, 0);
    //glRotatef(-angle - estirarDerecho * 80.0f, 1, 0, 0); // +80 grados si estirado
    glTranslatef(0,-14,0);
    {
        float cx =  -3.0f, cy = 9.0f;
        for(int i = 0; i < N; ++i) {
            float t0 = float(i)   / float(N);
            float t1 = float(i+1) / float(N);
            float a0 = M_PI/2 + (M_PI/2) * t0;
            float a1 = M_PI/2 + (M_PI/2) * t1;

            float x0 = cx + r * cosf(a0), y0 = cy + r * sinf(a0);
            float x1 = cx + r * cosf(a1), y1 = cy + r * sinf(a1);

            float dx = x1 - x0, dy = y1 - y0;
            float segLen = sqrtf(dx*dx + dy*dy);
            float theta  = atan2f(dy, dx) * 180.0f / M_PI;

            glPushMatrix();
                glTranslatef(x0, y0, zArm);
                glRotatef(theta, 0,0,1);
                glRotatef(-90.0f, 0,1,0);
                gluCylinder(quadric, armRad, armRad, segLen, 8, 1);
            glPopMatrix();
        }
    }
    Manos(-9.3,1);
    glPopMatrix();
}


void rueda(float x, float y, float z, float radio) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(0.1f, 0.1f, 0.1f); // negro
    glutSolidTorus(0.1 * radio, radio, 10, 20);
    glPopMatrix();
}

void tractor() {
    glPushMatrix();
        //glTranslatef(0.0, 1.6f, 10.0f);  // posición del tractor
        glTranslatef(tractorX, 1.6f, tractorZ);  // posición del tractor
        glScalef(1.0f, 1.0f, 1.0f);

        // Cuerpo inferior
        glColor3f(0.8f, 0.0f, 0.0f); // rojo
        glPushMatrix();
        glScalef(2.5f, 0.6f, 1.5f);
        glutSolidCube(1);
        glPopMatrix();

        // Cabina
        glColor3f(0.4f, 0.6f, 0.8f); // azul claro
        glPushMatrix();
        glTranslatef(0.2f, 0.8f, 0.0f);
        glScalef(1.2f, 1.0f, 1.0f);
        glutSolidCube(1);
        glPopMatrix();

        // Escape
        glColor3f(0.2f, 0.2f, 0.2f); // gris oscuro
        glPushMatrix();
        glTranslatef(1.0f, 1.2f, 0.0f);
        glRotatef(-90, 1, 0, 0);
        GLUquadric* quad = gluNewQuadric();
        gluCylinder(quad, 0.05, 0.05, 0.6, 10, 10);
        glPopMatrix();

        // Ruedas traseras (más grandes)
        rueda(-1.2f, -0.1f, -0.9f, 0.5f);
        rueda(-1.2f, -0.1f,  0.9f, 0.5f);

        // Ruedas delanteras (más pequeñas)
        rueda(1.3f, -0.1f, -0.9f, 0.3f);
        rueda(1.3f, -0.1f,  0.9f, 0.3f);

    glPopMatrix();
}

void nina(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // Cabeza
    glColor3f(1.0f, 0.8f, 0.6f);  // piel
    glPushMatrix();
    glTranslatef(0.0f, 1.8f, 0.0f);
    glutSolidSphere(0.3, 20, 20);
    glPopMatrix();

    // Cabello (semiesfera)
    glColor3f(0.4f, 0.2f, 0.1f);  // marrón
    glPushMatrix();
    glTranslatef(0.0f, 1.85f, 0.0f);
    glScalef(1.0, 0.6, 1.0);
    glutSolidSphere(0.35, 20, 20);
    glPopMatrix();

    // Cuerpo (vestido)
    glColor3f(1.0f, 0.4f, 0.7f);  // rosa
    glPushMatrix();
    glTranslatef(0.0f, 0.8f, 0.0f);
    glRotatef(-90, 1, 0, 0);
    glutSolidCone(0.4, 0.9, 20, 20);
    glPopMatrix();

    // Brazos
    glColor3f(1.0f, 0.8f, 0.6f);
    glPushMatrix();
    glTranslatef(-0.4f, 1.4f, 0.0f);
    glRotatef(90, 0, 1, 0);
    GLUquadric* qobj = gluNewQuadric();
    gluCylinder(qobj, 0.07, 0.07, 0.4, 10, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.4f, 1.4f, 0.0f);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(qobj, 0.07, 0.07, 0.4, 10, 10);
    glPopMatrix();

    // Piernas
    glColor3f(0.2f, 0.2f, 0.9f);  // azul oscuro
    glPushMatrix();
    glTranslatef(-0.15f, 0.55f, 0.0f);
    glScalef(0.1f, 0.6f, 0.1f);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.15f, 0.55f, 0.0f);
    glScalef(0.1f, 0.6f, 0.1f);
    glutSolidCube(1);
    glPopMatrix();

    // Zapatos
    glColor3f(0.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(-0.15f, 0.25f, 0.0f);
    glutSolidSphere(0.08, 10, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.15f, 0.25f, 0.0f);
    glutSolidSphere(0.08, 10, 10);
    glPopMatrix();

    glPopMatrix();
}

void rioAnimado() {
    glPushMatrix();
    glTranslatef(34.0f, 0.0f, rioOffset); // Desplazamiento de la textura simulada

    glColor3f(0.2f, 0.4f, 1.0f); // Azul agua
    glBegin(GL_QUADS);
    glVertex3f(-5, 0.51, -35);
    glVertex3f(5, 0.51, -35);
    glVertex3f(5, 0.51, 30);
    glVertex3f(-5, 0.51, 30);
    glEnd();

    glPopMatrix();
}


void dibujarPersonaje(){
    glPushMatrix();
        glTranslatef(0, 0.6, personajeZ);
        glRotatef(M_PI * 180.0 / M_PI, 0, 1, 0);
         // --- Ángulos de animación para brazos y piernas ---
        float anguloMiembros = sin(faseCaminata) * 26.0f;

        glScalef(0.2, 0.3, 0.5);
        Cabeza();
        //Manos(-9.3, 1);
        //Manos(9.3, -1);
        if(pateando){
            float anguloPateo = sin(fasePateo) * 90.0f;
            if(fasePateo > M_PI) {
                pateando = false; fasePateo = 0; impulsoAplicado = false;
            }
            //Brazos(anguloMiembros);
            Brazos(anguloMiembros, 0.0f);
            piernaPateo(2.3,anguloPateo);
        }
        else{
            //Brazos(anguloMiembros);
            Brazos(anguloMiembros, estirarBrazoDerecho);
            pierna(2.3,anguloMiembros);
        }
        glColor3ub(80, 80, 160);
        cintura();
        glColor3ub(60, 100, 200);
        torso();
        glColor3ub(120, 120, 120);
        cuello();
    glPopMatrix();
}

void dibujar(){
    inicializarLuces();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camX, camY, camZ,
          camX + dirX, camY + dirY, camZ + dirZ,
          0.0f, 1.0f, 0.0f);
    glClearColor(158.0 / 245.0, 212.0 / 245.0, 105.0 / 245.0, 4.0);

    glPushMatrix();
        cieloRasoColor();
        dibujarCielo();
        dibujarArbolesNavidad();

        //dibujarEjes();

        glPushMatrix();
            glScalef(1.5, 1.5, 1.0);
            dibujarCasaRustica();
        glPopMatrix();

        pasto();
        paisaje();

        dibujarNube(20, 30, -15);
        dibujarNube(-25, 28, 0);
        dibujarNube(5, 32, 20);

        glPushMatrix();
            glTranslatef(-10, 0, 15);
            glRotatef(45, 0, 1, 0);
            dibujarBanca();
            glPopMatrix();

            glPushMatrix();
            glTranslatef(15, 0, -10);
            glRotatef(-30, 0, 1, 0);
            dibujarBanca();
        glPopMatrix();

        sendero();

        dibujarPersonaje();
        tractor();

        glPushMatrix();
            glScalef(1.2, 1.2, 1.3);
            nina(5.0f, 0.5f, 12.0f); // posición en el sendero o jardín
        glPopMatrix();

        rioAnimado();
    glPopMatrix();

    glutSwapBuffers();
}


void moverCamara(unsigned char tecla, int x, int y) {
    float lateralX = -dirZ;
    float lateralZ = dirX;

    switch (tecla) {
        case 'w': case 'W':
            camX += dirX * velocidadCamara;
            camZ += dirZ * velocidadCamara;
            break;
        case 's': case 'S':
            camX -= dirX * velocidadCamara;
            camZ -= dirZ * velocidadCamara;
            break;
        case 'a': case 'A':
            camX -= lateralX * velocidadCamara;
            camZ -= lateralZ * velocidadCamara;
            break;
        case 'd': case 'D':
            camX += lateralX * velocidadCamara;
            camZ += lateralZ * velocidadCamara;
            break;
        case 'q': case 'Q':
            camY += velocidadCamara;
            break;
        case 'e': case 'E':
            camY -= velocidadCamara;
            break;

        case 'r': case 'R':
            personajeX = 0.0f;
            personajeZ = 20.0f;
            personajeAngulo = M_PI;
            velocidad = 0.05f;

            faseCaminata = 0.0f;
            pateando = false;
            fasePateo = 0.0f;
            enPausaParaPatear = false;
            contadorPausa = 0;
            haciaElFinal = true;

            tractorX = -1.0f;
            tractorZ = 0.0f;
            tractorVelocidadX = 0.0f;
            tractorVelocidadZ = 0.0f;
            impulsoAplicado = false;

            inclinacionCabeza = 0.0f;
            estirarBrazoDerecho = 0.0f;

            puertaAngulo = 0.0f;
            abriendoPuerta = false;
            cerrandoPuerta = false;
            robotEntrando = false;
            contadorEsperaPuerta = 0;
            break;
    }

    glutPostRedisplay();
}


void teclado_especial(int tecla, int x, int y){
    switch(tecla){
        case GLUT_KEY_LEFT:
            anguloCamara -= 0.05f;
            break;
        case GLUT_KEY_RIGHT:
            anguloCamara += 0.05f;
            break;
    }
    // Actualiza dirección de la cámara
    dirX = sin(anguloCamara);
    dirZ = -cos(anguloCamara);
    glutPostRedisplay();
}

//TIMER
void timer(int t) {
    float friccion = 0.98f;
    const int DURACION_PAUSA = 50; // Duración de la pausa (50 fotogramas ≈ 1 segundo)
    tractorX += tractorVelocidadX;
    tractorZ += tractorVelocidadZ;
    tractorVelocidadX *= friccion;
    tractorVelocidadZ *= friccion;

    if (sqrt(tractorVelocidadX * tractorVelocidadX + tractorVelocidadZ * tractorVelocidadZ) < 0.01) {
        tractorVelocidadX = 0;
        tractorVelocidadZ = 0;
    }

    // 2. LÓGICA DE LA ANIMACIÓN DE PATADA
    if (pateando) {
        fasePateo += 0.15;
        if (fasePateo > M_PI / 2.0 && !impulsoAplicado) {
            float fuerzaPatada = 0.4f;
            tractorVelocidadZ = 0;
            tractorVelocidadX = -fuerzaPatada;
            impulsoAplicado = true;
        }
    }

    // 3. LÓGICA DE MOVIMIENTO Y GIRO DEL PERSONAJE
    // Cambia el punto final para que el robot se detenga frente a la casita
    float puntoFinalZ_casita = -14.3f; // Ajusta según la posición de tu casa
    if (haciaElFinal) {
        if (!enPausaParaPatear && personajeZ > puntoFinalZ_casita) {
            personajeZ -= velocidad;
        } else if (personajeZ <= puntoFinalZ_casita) {
            haciaElFinal = false; // Se detiene al llegar a la casita
            estirarBrazoDerecho = 1.0f; // Estira el brazo derecho
        }

        if (abs(personajeZ - tractorZ) < 1.5f && abs(personajeX - tractorX) < 1.5f && !pateando && !enPausaParaPatear) {
            enPausaParaPatear = true;
        }
    }

    // 4. LÓGICA PARA LA PAUSA E INCLINACIÓN DE CABEZA
    if (enPausaParaPatear) {
        contadorPausa++;
        // Inclina la cabeza hacia abajo progresivamente (hasta -25 grados)
        if (inclinacionCabeza < 20.0f)
            inclinacionCabeza += 2.0f;

        if (contadorPausa >= DURACION_PAUSA) {
            enPausaParaPatear = false;
            pateando = true;
            fasePateo = 0;
            contadorPausa = 0;
        }
    } else {
        // Regresa la cabeza a su posición original suavemente
        if (inclinacionCabeza < 0.0f)
            inclinacionCabeza += 2.0f;
        if (inclinacionCabeza > 0.0f)
            inclinacionCabeza = 0.0f;
    }

    // 5. Actualizar animación de caminata
    if (haciaElFinal && !enPausaParaPatear) {
        faseCaminata += velocidad * 2.0;
    }

    // --- ANIMACIÓN DE PUERTA Y ENTRADA DEL ROBOT ---
    if (estirarBrazoDerecho == 1.0f && !abriendoPuerta && !robotEntrando && !cerrandoPuerta) {
        abriendoPuerta = true; // Inicia animación de apertura
    }

    if (abriendoPuerta) {
        if (puertaAngulo < 90.0f) {
            puertaAngulo += 3.0f; // Abre la puerta
            if (puertaAngulo > 90.0f) puertaAngulo = 90.0f;
        } else {
            abriendoPuerta = false;
            robotEntrando = true; // Ahora el robot entra
        }
    }

    if (robotEntrando) {
        // Mueve el robot hacia adentro (ajusta el valor de -18.5f según la posición de la puerta)
        if (personajeZ > -20.5f) {
            personajeZ -= 0.08f; // Entra lentamente
        } else {
            robotEntrando = false;
            contadorEsperaPuerta = 0;
            cerrandoPuerta = true; // Inicia cierre de puerta
        }
    }

    if (cerrandoPuerta) {
        contadorEsperaPuerta++;
        if (contadorEsperaPuerta > 30) { // Espera un poco antes de cerrar
            if (puertaAngulo > 0.0f) {
                puertaAngulo -= 3.0f; // Cierra la puerta
                if (puertaAngulo < 0.0f) puertaAngulo = 0.0f;
            } else {
                cerrandoPuerta = false;
                estirarBrazoDerecho = 0.0f; // Opcional: baja el brazo después de cerrar
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(20, timer, 0);
}
//Fin de Funcion


int main(int argc, char* argv[]){
    glutInit(&argc, argv);
    srand(time(NULL)); // <-- AÑADE ESTA LÍNEA AQUÍ
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(1400,900);
    glutInitWindowPosition(0,0);
    glutCreateWindow("Proyecto Final");
    quadric = gluNewQuadric();
    glutReshapeFunc(iniciarVentana);
    glutDisplayFunc(dibujar);
    glutKeyboardFunc(moverCamara);
    glutSpecialFunc(teclado_especial);
    glutTimerFunc(0,timer,0);
    glutMainLoop();

    return 0;
}
