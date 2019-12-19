#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}
float i = 0, j = 15;

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'W')
	{
		j += 1;
	}
	if (key == 'S')
	{
		if (j > 12)
			j -= 1;
	}
	if (key == 'A')
	{
		i += 0.1;
	}
	if (key == 'D')
	{
		i -= 0.1;
	}
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}




void cyl(GLfloat radius, GLfloat height)
{
	GLfloat x = 0.0;
	GLfloat y = 0.0;
	GLfloat a = 0.0;
	GLfloat a_stepsize = 0.1;

	/** Draw the tube */
	//glEnable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, texId);
	glPushMatrix();
	glBegin(GL_QUAD_STRIP);
	a = 0.0;
	glColor3d(0.6, 0.3, 0);
	while (a < 2 * PI) {
		x = radius * cos(a);
		y = radius * sin(a);
		glNormal3f(0, y, x);
		const float tc = (a / (float)(2 * PI));
		glTexCoord2f(tc, 1.0);
		glVertex3f(height, y, x);
		glTexCoord2f(tc, 0.0);
		glVertex3f(0.0, y, x);
		a = a + a_stepsize;
	}
	glNormal3f(0, y, x);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(height, 0.0, radius);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.0, 0.0, radius);
	glEnd();
	

	glBegin(GL_POLYGON);
	a = 0.0;
	glColor3d(0.5, 0.2, 0.1);
	glNormal3b(1, 0, 0);
	while (a < 2 * PI) {
		x = radius * cos(a);
		y = radius * sin(a);
		glVertex3f(height, y, x);
		a = a + a_stepsize;
	}
	glVertex3f(height, 0.0, radius);
	glEnd();
	glBegin(GL_POLYGON);
	a = 0.0;
	glColor3d(0.5, 0.2, 0.1);
	glNormal3b(-1, 0, 0);
	while (a < 2 * PI) {
		x = radius * cos(a);
		y = radius * sin(a);
		glVertex3f(0.0, y, x);
		a = a + a_stepsize;
	}
	glVertex3f(0.0, 0.0, radius);
	glEnd();
	glPopMatrix();

}

void okn()
{
	glBegin(GL_QUADS);

	{glPushMatrix();

	glColor3d(0.5, 0.2, 0.1);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, 0.8);
	glVertex3d(0, 0.2, 0.8);
	glVertex3d(0, 0.2, 0);

	glVertex3d(0, 0, 0.8);
	glVertex3d(1, 0, 0.8);
	glVertex3d(1, 0.2, 0.8);
	glVertex3d(0, 0.2, 0.8);

	glVertex3d(1, 0, 0.8);
	glVertex3d(1, 0, 0);
	glVertex3d(1, 0.2, 0);
	glVertex3d(1, 0.2, 0.8);
	glVertex3d(0, 0, 0);
	glVertex3d(1, 0, 0);
	glVertex3d(1, 0.2, 0);
	glVertex3d(0, 0.2, 0);

	glColor4d(0.9, 0.9, 1, 0.1);
	glNormal3d(0, -1, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, 0.8);
	glVertex3d(1, 0, 0.8);
	glVertex3d(1, 0, 0);
	glNormal3d(0, 1, 0);
	glVertex3d(0, 0.2, 0);
	glVertex3d(0, 0.2, 0.8);
	glVertex3d(1, 0.2, 0.8);
	glVertex3d(1, 0.2, 0);
	glPopMatrix();
	}

	glEnd();

}


typedef struct _Position
{
	GLfloat X, Y, Z;
}TPosition;
TPosition Position;
GLfloat Move = 0.0, t1 = 0.0;
GLfloat d = 0.0, f = 0.0;



void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  
	glPushMatrix();
	glTranslated(0, 0, -0.2);
	for (int i = 0; i < 16; i++)
	{
		glTranslated(0, 0, 0.4);
		if (i < 6) {
			glPushMatrix();
			cyl(0.2, 2);
			glTranslated(3, 0, 0);
			cyl(0.2, 2);
			glPopMatrix();
		}
		else
			cyl(0.2, 5);
		glPushMatrix();

		glRotated(120, 0, 0, 1);
		glTranslated(-0.9, -0.5, 0);
		if ((9 < i) && (i < 14)) {
			glPushMatrix();
			cyl(0.2, 2);
			glTranslated(3, 0, 0);
			cyl(0.2, 2);
			glPopMatrix();
		}
		else
			cyl(0.2, 5);
		glPopMatrix();
		glPushMatrix();
		glTranslated(4.1, -0.5, 0);
		glRotated(60, 0, 0, 1);
		if ((9 < i) && (i < 14)) {
			glPushMatrix();
			cyl(0.2, 2);
			glTranslated(3, 0, 0);
			cyl(0.2, 2);
			glPopMatrix();
		}
		else
			cyl(0.2, 5);
		glPopMatrix();

		glPushMatrix();
		glTranslated(5, 6.5, 0);
		glRotated(180, 0, 0, 1);

		glPushMatrix();
		if ((9 < i) && (i < 14)) {
			glPushMatrix();
			cyl(0.2, 2);
			glTranslated(3, 0, 0);
			cyl(0.2, 2);
			glPopMatrix();
		}
		else
			cyl(0.2, 5);
		glRotated(120, 0, 0, 1);
		glTranslated(-0.9, -0.5, 0);
		if ((1 < i) && (i < 6)) {
			glPushMatrix();
			cyl(0.2, 2);
			glTranslated(3, 0, 0);
			cyl(0.2, 2);
			glPopMatrix();
		}
		else
			cyl(0.2, 5);
		glPopMatrix();
		glPushMatrix();
		glTranslated(4.1, -0.5, 0);
		glRotated(60, 0, 0, 1);
		if ((1 < i) && (i < 6)) {
			glPushMatrix();
			cyl(0.2, 2);
			glTranslated(3, 0, 0);
			cyl(0.2, 2);
			glPopMatrix();
		}
		else
			cyl(0.2, 5);
		glPopMatrix();

		glPopMatrix();
	}
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	{
		double A[] = { -1.4, 3.3,0 };
		double B[] = { 0.5, 0 ,0 };
		double C[] = { 4.5, 0,0 };
		double D[] = { 6.4, 3.3,0 };
		double E[] = { 0.5, 6.6 ,0 };
		double F[] = { 4.5, 6.6,0 };
		double O[] = { 2.5, 3.3,3 };
		glBegin(GL_QUADS);
		glVertex3dv(A);
		glVertex3dv(B);
		glVertex3dv(C);
		glVertex3dv(D);

		glVertex3dv(A);
		glVertex3dv(E);
		glVertex3dv(F);
		glVertex3dv(D);
		glEnd();
		glPushMatrix();
		glTranslated(0, 0, 3.2);
		glBegin(GL_QUADS);
		glVertex3dv(A);
		glVertex3dv(B);
		glVertex3dv(C);
		glVertex3dv(D);

		glVertex3dv(A);
		glVertex3dv(E);
		glVertex3dv(F);
		glVertex3dv(D);
		glEnd();
		glPopMatrix();
		glPushMatrix();
		glTranslated(0, 0, 6.4);
		glBegin(GL_TRIANGLES);
		glColor3d(0, 0.2, 0.1);
		glNormal3b(-1, 0.58, -1.3);

		glVertex3dv(A);
		glVertex3dv(B);
		glVertex3dv(O);
		glNormal3b(1, -0.58, 1.3);
		glVertex3dv(D);
		glVertex3dv(C);
		glVertex3dv(O);
		glNormal3b(0, 1.1, 1);
		glVertex3dv(E);
		glVertex3dv(F);
		glVertex3dv(O);
		glColor3d(0.2, 0, 0.1);
		glNormal3b(-1, -0.58, -1.3);
		glVertex3dv(O);
		glVertex3dv(E);
		glVertex3dv(A);
		glNormal3b(1, 0.58, 1.3);
		glVertex3dv(O);
		glVertex3dv(D);
		glVertex3dv(F);
		glNormal3b(0, -1.1, 1);
		glTexCoord2d(0, 0);
		glVertex3dv(B);
		glTexCoord2d(1, 0);
		glVertex3dv(C);
		glTexCoord2d(0.5, 0.5);
		glVertex3dv(O);
		glEnd();
		glPopMatrix(); }
	glPushMatrix();

	{glPushMatrix();
	glTranslated(2, 0, 0);

	if (d < 1)
	{
		d += 0.01; f = 0;
	}
	if (d >= 1)
	{
		d = 1; f += 0.01;
		if (f > 1)
		{
			d = 0; f = 0;
		}
	}

	glRotated(90 * (d - f), 0, 0, 1);

	{glBegin(GL_QUADS);

	glColor3d(0.4, 0.2, 0.2);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, 2.4);
	glVertex3d(0, 0.2, 2.4);
	glVertex3d(0, 0.2, 0);

	glVertex3d(0, 0, 2.4);
	glVertex3d(1, 0, 2.4);
	glVertex3d(1, 0.2, 2.4);
	glVertex3d(0, 0.2, 2.4);

	glVertex3d(1, 0, 2.4);
	glVertex3d(1, 0, 0);
	glVertex3d(1, 0.2, 0);
	glVertex3d(1, 0.2, 2.4);
	glVertex3d(0, 0, 0);
	glVertex3d(1, 0, 0);
	glVertex3d(1, 0.2, 0);
	glVertex3d(0, 0.2, 0);
	glNormal3d(0, -1, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, 2.4);
	glVertex3d(1, 0, 2.4);
	glVertex3d(1, 0, 0);
	glNormal3d(0, 1, 0);
	glVertex3d(0, 0.2, 0);
	glVertex3d(0, 0.2, 2.4);
	glVertex3d(1, 0.2, 2.4);
	glVertex3d(1, 0.2, 0);
	glEnd();
	glPopMatrix(); }}

	{glPushMatrix();
	glColor4d(0.9, 0.9, 1, 0.1);
	glRotated(120, 0, 0, 1);
	glTranslated(-0.9, -0.5, 0);
	glTranslated(2, 0, 4.8);
	okn();
	glTranslated(0, 0, -0.8);
	if (Move < 0.8)
	{
		Move += 0.001;
		Position.Z = Move;
	}
	if (Move > 0.8)
	{
		Position.Z -= 0.001;
		if (Position.Z < 0)
			Move = 0;
	}
	glTranslated(Position.X, Position.Y, Position.Z);
	okn();
	glPopMatrix();

	glPushMatrix();
	glTranslated(4.1, -0.5, 0);
	glRotated(60, 0, 0, 1);
	glTranslated(2, 0, 4.8);
	okn();
	glTranslated(0, 0, -0.8);
	if (Move < 0.8)
	{
		Move += 0.001;
		Position.Z = Move;
	}
	if (Move > 0.8)
	{
		Position.Z -= 0.001;
		if (Position.Z < 0)
			Move = 0;
	}
	glTranslated(Position.X, Position.Y, Position.Z);
	okn();
	glPopMatrix();
	glTranslated(5, 6.5, 0);
	glRotated(180, 0, 0, 1);
	glPushMatrix();
	glTranslated(2, 0, 4.8);
	okn();
	glTranslated(0, 0, -0.8);
	if (Move < 0.8)
	{
		Move += 0.001;
		Position.Z = Move;
	}
	if (Move > 0.8)
	{
		Position.Z -= 0.001;
		if (Position.Z < 0)
			Move = 0;
	}
	glTranslated(Position.X, Position.Y, Position.Z);
	okn();
	glPopMatrix();
	glPushMatrix();
	glRotated(120, 0, 0, 1);
	glTranslated(-0.9, -0.5, 0);
	glTranslated(2, 0, 1.6);
	okn();
	glTranslated(0, 0, -0.8);
	if (Move < 0.8)
	{
		Move += 0.001;
		Position.Z = Move;
	}
	if (Move > 0.8)
	{
		Position.Z -= 0.001;
		if (Position.Z < 0)
			Move = 0;
	}
	glTranslated(Position.X, Position.Y, Position.Z);
	okn();
	glPopMatrix();
	glPushMatrix();
	glTranslated(4.1, -0.5, 0);
	glRotated(60, 0, 0, 1);
	glTranslated(2, 0, 1.6);
	okn();
	glTranslated(0, 0, -0.8);
	if (Move < 0.8)
	{
		Move += 0.001;
		Position.Z = Move;
	}
	if (Move > 0.8)
	{
		Position.Z -= 0.001;
		if (Position.Z < 0)
			Move = 0;
	}
	glTranslated(Position.X, Position.Y, Position.Z);
	okn();
	glPopMatrix();
	glPopMatrix(); }


	glColor3d(0.9, 0.6, 0.1);
	glPushMatrix();
	t1 += 0.01;

	glRotated(90 * t1, cos(i), sin(i), 0);
	Sphere c, l;
	c.pos = { 2.5,3.3,j };
	c.scale = c.scale * 2;
	c.Show();
	glColor3d(0.8, 0.8, 0.9);
	l.pos = { 2.5,3.3,-j };
	l.scale = l.scale;
	l.Show();
	glPopMatrix();
	

   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}