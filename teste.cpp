#include <GL/freeglut.h>
#include <SOIL/SOIL.h>
#include <cmath>
#include <stdio.h>

GLuint skyboxTexture = 0, groundTexture = 0, ballTexture = 0;
// Posição da câmera
float camX = 0.0f, camY = 1.0f, camZ = 5.0f;
// Direção que a câmera está olhando
float dirX = 0.0f, dirY = 0.0f, dirZ = -1.0f;
// Ângulos para controle do mouse
float yaw = -90.0f, pitch = 0.0f;
// Sensibilidade do mouse
float sensibilidade = 0.1f;
// Velocidade do movimento
float velocidade = 0.1f;

float playerX, playerY, playerZ; // Posição do jogador (centro do corpo)
float pernaEsqX, pernaEsqY, pernaEsqZ;
float pernaDirX, pernaDirY, pernaDirZ;
float passo = 0.0f; // Controle do efeito de caminhada

// Variáveis globais para posição da bola
float pos_ballX = 0.0f, pos_ballY = 0.0f, pos_ballZ = -6.0f;
float ballRotationAngle = 0.0f;
float ballRadius = 0.2f;
float groundY = -1.0f;  // Altura do chão

bool isMoving = false;
bool ballHit = false;  // Indica se a bola foi atingida
float hitTimer = 0.0f; // Tempo restante do empurrão
float pushSpeed = 0.05f; // Velocidade do empurrão
float pushDirX = 0.0f, pushDirZ = 0.0f; // Direção do empurrão

// Dimensões da janela
int larguraJanela = 800, alturaJanela = 600;

bool checkCollisionWithGround(){
    return (pos_ballY - ballRadius <= groundY);
}

bool checkCollisionWithPlayer(){
    float dx = pernaEsqX - pos_ballX;
    float dy = pernaEsqY - pos_ballY;
    float dz = pernaEsqZ - pos_ballZ;
    float distance = sqrt(dx*dx + dy*dy + dz*dz);

    return (distance < ballRadius + 0.5); // Se a distância for menor que o raio da bola, há colisão
}

void BallPos(){
    pos_ballY -= 0.01f; // Simula gravidade (queda)

    if (checkCollisionWithGround()) {
        pos_ballY = groundY + ballRadius;
    }

    // Se a bola foi atingida, mova por um tempo
    if (ballHit) {
        pos_ballX += pushDirX * pushSpeed;
        pos_ballZ += pushDirZ * pushSpeed;

        hitTimer -= 0.05f; // Diminui o tempo do empurrão
        if (hitTimer <= 0.0f) {
            ballHit = false; // Para o movimento após o tempo acabar
        }
    }
}

void BallRot(){
    if (checkCollisionWithPlayer()) { 
        ballRotationAngle += 5.0f;  // Ajuste a velocidade da rotação
        
        if (ballRotationAngle >= 360.0f) {
            ballRotationAngle -= 360.0f;
        }

        // Calcula a direção do empurrão (inverso da posição relativa do player)
        float dx = pos_ballX - pernaEsqX;
        float dz = pos_ballZ - pernaEsqZ;
        float length = sqrt(dx*dx + dz*dz);

        if (length > 0.0f) {
            pushDirX = dx / length;
            pushDirZ = dz / length;
        } else {
            pushDirX = 0.0f;
            pushDirZ = 0.0f;
        }

        ballHit = true;
        hitTimer = 1.0f; // A bola se moverá por um curto período
    }
}

void load_texture(GLuint* texture, const char* image_location){ 
    if (!*texture) {
        *texture = SOIL_load_OGL_texture(image_location, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
        if (!*texture) {
            printf("Erro: '%s'", SOIL_last_result());
        }
    }

    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void init(){
    GLfloat lightenv[] = {0.2, 0.2, 0.2, 1.0};
    GLfloat lightColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lightPos[] = {2.0f, 12.0f, 0.0f, 1.0f};  // 2.0f, 0.0f, -5.0f

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHT0);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightenv);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightenv);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);

    glClearColor(0.1, 0.1, 0.1, 1.0);

    glEnable(GL_NORMALIZE);

    glutSetCursor(GLUT_CURSOR_NONE);
}

void Player() {
    glPushMatrix();
        glColor3f(0.3f, 0.2f, 0.1f); // Cor marrom para as pernas

        // Perna esquerda
        glPushMatrix();
            glTranslatef(pernaEsqX, pernaEsqY, pernaEsqZ);
            glScalef(0.2f, 1.0f, 0.2f);
            glutSolidCube(1.0);
        glPopMatrix();

        // Perna direita
        glPushMatrix();
            glTranslatef(pernaDirX, pernaDirY, pernaDirZ);
            glScalef(0.2f, 1.0f, 0.2f);
            glutSolidCube(1.0);
        glPopMatrix();

    glPopMatrix();
}

void PlayerPos() {
    playerX = camX;
    playerY = camY - 1.0f; // Ajusta a altura para o centro do corpo
    playerZ = camZ;

    // Define a posição das pernas em relação ao jogador
    float deslocamento = sin(passo) * 0.1f; // Alternância das pernas para caminhar

    pernaEsqX = playerX - 0.2f;
    pernaEsqY = playerY - 0.5f;
    pernaEsqZ = playerZ + deslocamento; // Efeito de caminhada

    pernaDirX = playerX + 0.2f;
    pernaDirY = playerY - 0.5f;
    pernaDirZ = playerZ - deslocamento; // Alternando para trás
}

void Sky(float tamanho){
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, skyboxTexture);

    float t = tamanho / 2.0f;
    glPushMatrix();
        glTranslatef(camX, camY, camZ);  // Move o cubo para sempre estar na posição da câmera

        glBegin(GL_QUADS);
        // Frente
        glTexCoord2f(0.25f, 0.33f); glVertex3f(-t, -t, -t);
        glTexCoord2f(0.50f, 0.33f); glVertex3f(t, -t, -t);
        glTexCoord2f(0.50f, 0.66f); glVertex3f(t, t, -t);
        glTexCoord2f(0.25f, 0.66f); glVertex3f(-t, t, -t);
        
        // Fundo
        glTexCoord2f(0.75f, 0.33f); glVertex3f(t, -t, t);
        glTexCoord2f(1.00f, 0.33f); glVertex3f(-t, -t, t);
        glTexCoord2f(1.00f, 0.66f); glVertex3f(-t, t, t);
        glTexCoord2f(0.75f, 0.66f); glVertex3f(t, t, t);
        
        // Esquerda
        glTexCoord2f(0.00f, 0.33f); glVertex3f(-t, -t, t);
        glTexCoord2f(0.25f, 0.33f); glVertex3f(-t, -t, -t);
        glTexCoord2f(0.25f, 0.66f); glVertex3f(-t, t, -t);
        glTexCoord2f(0.00f, 0.66f); glVertex3f(-t, t, t);
        
        // Direita
        glTexCoord2f(0.50f, 0.33f); glVertex3f(t, -t, -t);
        glTexCoord2f(0.75f, 0.33f); glVertex3f(t, -t, t);
        glTexCoord2f(0.75f, 0.66f); glVertex3f(t, t, t);
        glTexCoord2f(0.50f, 0.66f); glVertex3f(t, t, -t);
        
        // Cima
        glTexCoord2f(0.25f, 0.66f); glVertex3f(-t, t, -t);
        glTexCoord2f(0.50f, 0.66f); glVertex3f(t, t, -t);
        glTexCoord2f(0.50f, 1.00f); glVertex3f(t, t, t);
        glTexCoord2f(0.25f, 1.00f); glVertex3f(-t, t, t);
        
        // Baixo
        glTexCoord2f(0.25f, 0.00f); glVertex3f(-t, -t, -t);
        glTexCoord2f(0.50f, 0.00f); glVertex3f(t, -t, -t);
        glTexCoord2f(0.50f, 0.33f); glVertex3f(t, -t, t);
        glTexCoord2f(0.25f, 0.33f); glVertex3f(-t, -t, t);
        glEnd();
    glPopMatrix();

    glEnable(GL_LIGHTING);
}

void Chao(){

    glEnable(GL_TEXTURE_2D);  // Ativar texturas
    glBindTexture(GL_TEXTURE_2D, groundTexture);  // Vincular a textura

    glColor3f(1.0f, 1.0f, 1.0f);  // Branco para não alterar a textura
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, -1.0f, -10.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(10.0f, -1.0f, -10.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(10.0f, -1.0f, 10.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-10.0f, -1.0f, 10.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_QUADS);
        glVertex3f(-10.0f, -1.0f, -10.0f); 
        glVertex3f(10.0f, -1.0f, -10.0f);   
        glVertex3f(10.0f, 5.0f, -10.0f);    
        glVertex3f(-10.0f, 5.0f, -10.0f);   
    glEnd();
}

void Grid(){
    glColor3f(0.3f, 0.3f, 0.3f); 
    glBegin(GL_LINES);
    for (float i = -10; i <= 10; i += 1.0) {
        glVertex3f(i, -1.0f, -10.0f);
        glVertex3f(i, -1.0f, 10.0f);
        glVertex3f(-10.0f, -1.0f, i);
        glVertex3f(10.0f, -1.0f, i);
    }
    glEnd();
}

void Trave(){
    GLUquadric* quadric = gluNewQuadric();
    glColor3f(1.0, 1.0, 1.0);  // Cor branca da trave

    float altura = 2.5f;
    float largura = 3.0f;
    float espessura = 0.1f;

    glPushMatrix();
        // Poste esquerdo
        glTranslatef(-largura / 2, -0.8f, -7.0f);
        glRotatef(-90, 1, 0, 0);
        gluCylinder(quadric, espessura, espessura, altura, 20, 20);
    glPopMatrix();

    glPushMatrix();
        // Poste direito
        glTranslatef(largura / 2, -0.8f, -7.0f);
        glRotatef(-90, 1, 0, 0);
        gluCylinder(quadric, espessura, espessura, altura, 20, 20);
    glPopMatrix();

    glPushMatrix();
        // Barra transversal
        glTranslatef(-largura / 2, altura - 0.8f, -7.0f);
        glRotatef(90, 0, 1, 0);
        gluCylinder(quadric, espessura, espessura, largura, 20, 20);
    glPopMatrix();

    gluDeleteQuadric(quadric);
}

void Rede(){
    glColor3f(1.0f, 1.0f, 1.0f);  // Cor branca da rede

    float altura = 2.5f;
    float largura = 3.0f;
    float profundidade = 1.2f;  // Ajustado para cobrir melhor
    int segmentos = 10;  // Número de quadrados na malha

    glBegin(GL_LINES);
    
    // **Painel traseiro da rede**
    for (int i = 0; i <= segmentos; i++) {
        float y = i * (altura / segmentos) - 0.8f;  
        float xStart = -largura / 2;
        float xEnd = largura / 2;
        float z = -7.0f - profundidade;

        // Linhas verticais traseiras
        glVertex3f(xStart + i * (largura / segmentos), -0.8f, z);
        glVertex3f(xStart + i * (largura / segmentos), altura - 0.8f, z);

        // Linhas horizontais traseiras
        glVertex3f(xStart, y, z);
        glVertex3f(xEnd, y, z);
    }

    // **Laterais da rede**
    for (int i = 0; i <= segmentos; i++) {
        float y = i * (altura / segmentos) - 0.8f;
        float zFront = -7.0f;
        float zBack = -7.0f - profundidade;

        // Lado esquerdo
        glVertex3f(-largura / 2, y, zFront);
        glVertex3f(-largura / 2, y, zBack);

        // Lado direito
        glVertex3f(largura / 2, y, zFront);
        glVertex3f(largura / 2, y, zBack);
    }

    // **Linhas de profundidade para conectar a rede**
    for (int i = 0; i <= segmentos; i++) {
        float z = -7.0f - (i * (profundidade / segmentos));
        float xStart = -largura / 2;
        float xEnd = largura / 2;
        float yTop = altura - 0.8f;
        float yBottom = -0.8f;

        // Linhas superiores (conectam barra transversal à rede traseira)
        glVertex3f(xStart, yTop, z);
        glVertex3f(xEnd, yTop, z);
    }

    glEnd();
}

void Ball(){
    GLUquadric* quadric = gluNewQuadric();  

    glPushMatrix();
        glEnable(GL_TEXTURE_2D);  
        glBindTexture(GL_TEXTURE_2D, ballTexture);  

        gluQuadricTexture(quadric, GL_TRUE);  
        glColor3f(1.0, 1.0, 1.0);  
        glTranslatef(pos_ballX, pos_ballY, pos_ballZ);
        glRotatef(ballRotationAngle, 0, 1, 0);
        gluSphere(quadric, ballRadius, 30, 30);  

        glDisable(GL_TEXTURE_2D);  
    glPopMatrix();

    gluDeleteQuadric(quadric);  
}

void mouseMotion(int x, int y) {
    int centroX = larguraJanela / 2;
    int centroY = alturaJanela / 2;

    if (x == centroX && y == centroY) return;

    float offsetX = (x - centroX) * sensibilidade;
    float offsetY = (centroY - y) * sensibilidade;

    yaw += offsetX;
    pitch += offsetY;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    float radYaw = yaw * M_PI / 180.0f;
    float radPitch = pitch * M_PI / 180.0f;
    
    dirX = cos(radYaw) * cos(radPitch);
    dirY = sin(radPitch);
    dirZ = sin(radYaw) * cos(radPitch);

    glutWarpPointer(centroX, centroY);
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    float moveX = dirX * velocidade;
    float moveZ = dirZ * velocidade;
    switch (key) {
        case 'w':
            camX += moveX;
            camZ += moveZ;
            isMoving = true;
            break;
        case 's':
            camX -= moveX;
            camZ -= moveZ;
            isMoving = true;
            break;
        case 'a':
            camX += moveZ;
            camZ -= moveX;
            isMoving = true;
            break;
        case 'd':
            camX -= moveZ;
            camZ += moveX;
            isMoving = true;
            break;
        case 27:
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    gluLookAt(camX, camY, camZ, 
              camX + dirX, camY + dirY, camZ + dirZ, 
              0.0f, 1.0f, 0.0f);

    Chao();
    Grid();
    Sky(50.0f);
    Ball();
    BallRot();
    BallPos();
    Player();
    if (isMoving) {
        passo += 0.2f; // Atualiza o efeito de caminhada
    }
    PlayerPos();

    glPushMatrix();
        glDisable(GL_LIGHTING);  // Desativa a iluminação para a esfera da luz
        glColor3f(1.0f, 1.0f, 1.0f);  // Amarelo para representar a luz
        glTranslatef(2.0f, 12.0f, 0.0f);  // Mesma posição da luz
        glutSolidSphere(0.5, 20, 20);  // Esfera pequena representando a luz
        glEnable(GL_LIGHTING);  // Reativa a iluminação
    glPopMatrix();
    
    Trave();
    Rede();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    larguraJanela = w;
    alturaJanela = h;
    
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / (float)h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);

    glutWarpPointer(larguraJanela / 2, alturaJanela / 2);
}

void update(int value) {
    BallRot();
    BallPos();
    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // Atualiza a cada ~16ms (aproximadamente 60 FPS)
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(larguraJanela, alturaJanela);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Movimentacao FPS com OpenGL");

    init();
    //load_texture(&skyboxTexture, "ceu.png");
    load_texture(&ballTexture, "ball.jpg");
    load_texture(&groundTexture, "glass.jpg");
    glutFullScreen();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(mouseMotion);

    glutWarpPointer(larguraJanela / 2, alturaJanela / 2);

    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
