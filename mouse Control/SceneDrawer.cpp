                                                                     
                                                                     
                                                                     
                                             
/*****************************************************************************
*                                                                            *
*  OpenNI 1.0 Alpha                                                          *
*  Copyright (C) 2010 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  OpenNI is free software: you can redistribute it and/or modify            *
*  it under the terms of the GNU Lesser General Public License as published  *
*  by the Free Software Foundation, either version 3 of the License, or      *
*  (at your option) any later version.                                       *
*                                                                            *
*  OpenNI is distributed in the hope that it will be useful,                 *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
*  GNU Lesser General Public License for more details.                       *
*                                                                            *
*  You should have received a copy of the GNU Lesser General Public License  *
*  along with OpenNI. If not, see <http://www.gnu.org/licenses/>.            *
*                                                                            *
*****************************************************************************/




//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "SceneDrawer.h"

#include <WinUser.h>		// Header File For Windows
//#include <winsock2.h>
//include <ws2bth.h>
//#include <BluetoothAPIs.h>
#include <stdlib.h>
#include <stdio.h>
#include<string.h>
//#include <BluetoothAPIs.h>
//#include <iostream>
#include<string.h>
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <OCIdl.h>
//#pragma comment(lib, "ws2_32.lib")
//#pragma comment(lib, "irprops.lib")
//using namespace std;
#include<math.h>
#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

extern xn::UserGenerator g_UserGenerator;

extern xn::DepthGenerator g_DepthGenerator;
xn::SceneMetaData scene;

extern XnBool g_bDrawBackground;
extern XnBool g_bDrawPixels;
extern XnBool g_bDrawSkeleton;
extern XnBool g_bPrintID;
extern XnBool g_bPrintState;
static int PreZ;
static int PreX;
static int PreY;
static int PZ;
static int PX;
static int PY;
static int tot=0;
static int tot2=0;
//static int i=0;
//static int j=0;
static int state=0; //left hand status
static int stater=0; //right hand satus
static int adwsflag; // left right up down gesture flag
static int shiftkflag; // shift keys
static int writing[11];
char chwrite[6];
HDC			hDC=NULL;		// Private GDI Device Context
HGLRC		hRC=NULL;		// Permanent Rendering Context
HWND		hWnd=NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application
//

HANDLE Input;
//

static int avgX[20];
static int avgY[20];
static int avgZ[20];
#define MAX_DEPTH 10000
static int charnum=0;
float g_pDepthHist[MAX_DEPTH];
GLYPHMETRICSFLOAT gmf[256];	// Storage For Information About Our Outline Font Characters
GLuint	texture[30];			// Storage For One Texture ( NEW )
GLuint	base;				// Base Display List For The Font Set
char names[][6]={"hesam","he","many","wyatt","h"};
char found[5][6];



//new code

void handleinit(HANDLE x)
{
	Input=x;
}
void ProcessNewHandPos(int NewX, int NewY);
unsigned int getClosestPowerOfTwo(unsigned int n)
{
	unsigned int m = 2;
	while(m < n) m<<=1;

	return m;
}
GLuint initTexture(void** buf, int& width, int& height)
{
	GLuint texID = 0;
	glGenTextures(1,&texID);

	width = getClosestPowerOfTwo(width);
	height = getClosestPowerOfTwo(height); 
	*buf = new unsigned char[width*height*4];
	glBindTexture(GL_TEXTURE_2D,texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return texID;
}

GLfloat texcoords[8];
void DrawRectangle(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY)
{
	GLfloat verts[8] = {	topLeftX, topLeftY,
		topLeftX, bottomRightY,
		bottomRightX, bottomRightY,
		bottomRightX, topLeftY
	};
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	//TODO: Maybe glFinish needed here instead - if there's some bad graphics crap
	glFlush();
}
void DrawTexture(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY)
{
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

	DrawRectangle(topLeftX, topLeftY, bottomRightX, bottomRightY);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

XnFloat Colors[][3] =
{
	{0,1,1},
	{0,0,1},
	{0,1,0},
	{1,1,0},
	{1,0,0},
	{1,.5,0},
	{.5,1,0},
	{0,.5,1},
	{.5,0,1},
	{1,1,.5},
	{1,1,1}
};
XnUInt32 nColors = 10;

void glPrintString(void *font, char *str)
{
	int i,l = strlen(str);

	for(i=0; i<l; i++)
	{
		glutBitmapCharacter(font,*str++);
	}
}
int depht(int x ,int y)
{
	int z=0;
	const XnDepthPixel* pDepthMap = g_DepthGenerator.GetDepthMap();
	z=((int) pDepthMap[(((int)y) * 640) +((int)x)]);
	return z;
}
void drawcircle(double r,double x,double y)
{
    glBegin(GL_POLYGON);//Start drawing a polygon
	for(int i=0; i<=360; i++)
	{
		 glColor3f(1,0,0);
         x=x+(r* cos(i*3.14/180));
		 glColor3f(0+r,0,1);
		 y=y+(r* sin(i*3.14/180));
		 glVertex2f(x,y);//last vertex
	}
	glEnd();
}

int avg(int a,int b)
{
	int avg=0;
	avg=(abs(a)+abs(b))/2;
	return avg;
}

void detect(int x,int y ,int z)
{
	int NewZ=z;
	int NewX=x;
	int NewY=y;
	int dZ = NewZ - PZ;
	int dX = NewX - PX;
	int dY = NewY - PY;
    static int f1=0;
	static int f2=0;
	int static ZX;
	int static ZY;
	int static ZZ;
	int static flag=0;
	int static flag2=0;

	PZ = NewZ;
	PX = NewX;
	PY = NewY;
	dZ=abs(dZ);
	dY=abs(dY);
	dX=abs(dX);

	if(dZ<=1 && dY<=1 && dX<=1)
	{
		//printf("Steady state\n");
		ZX=x;
		ZY=y;
		ZZ=z;

	}
	else
	{
		//tot2=((int) pDepthMap[(((int)ZY) * 640) +((int)ZX)])-ZZ;
		tot2=depht((int)ZX,(int)ZY)-ZZ;
		//printf("\nthis the prev Z = %d = ? = %d\n",(int) pDepthMap[(((int)ZY) * 640) +((int)ZX)],ZZ);
	}
	
	if((tot2>=1000))
	{
		
		flag=1;
		//printf("over 2000");
	}
		
	if(tot2<=200 && tot2>=-200 )
	{
		flag2=1;
		
	}
	if(flag2==1 && flag==1)
	{
		
		if(f1==0)
		{
			f1=1;
			//printf("\nf1 was 0  \n");
			//mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		}
		else if(f1==1)
		{
			//f2++;
			printf("\nyesssssssssssssss %d \n",f2);
			/*if(f2==0)
			{
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				f2=1;
				
			}
			else if(f2==1)
			{
				mouse_event(MOUSEEVENTF_LEFTUP,  0, 0,0, 0);
				f2=0;
			}

			//mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			//mouse_event(MOUSEEVENTF_LEFTUP,  0, 0,0, 0);*/
			f1=0;

		}
		flag2=0; flag=0;
	}
	
	
	
	
	
 }

int distance(double x1,double x2,double y1,double y2,double z1,double z2,int dimention)
{
	
	double bd;

	if(dimention==2)
	{
		bd=(pow((x1-x2),2)+pow((y1-y2),2));
		
	}
	else if(dimention==3)
	{
		bd=(pow((x1-x2),2)+pow((y1-y2),2)+pow((z1-z2),2));
	}
	else;
	return(sqrt(bd));

}
double slope(int x1,int x2,int y1,int y2)
{
	double m;
	m=((y2-y1)/(x2-x1));
	return m;

}
int bvalue(int x,int y,int m)
{
	int b;
	b=y-(m*x);
	return b;
}
void yvalue(double x1,double x2,double m,double b)
{
       double y1=(m*x1)+b;
       double y2=(m*x2)+b;
       printf("(Y1)=%lf\n",y1);
       printf("(Y2)=%lf\n",y2);
}
void calculate(double d,double x2,double y2,double x1,double y1)
{
    double yt,xt;
	double m, b;
    yt= y2-y1;
    xt= x2-x1;
    if(yt==0)
    {
      printf("(Y)=%lf and new(X)=%lf\n",y2,x2+d);
      return;       
    }
    else if(xt==0)
    {
      printf("(Y)=%lf and new(X)=%lf\n",y2+d,x2);
      return;   
    }
    m=((yt)/(xt));
    b=y2-(m*x2);
    double xp=pow(x2,2);
    double yp=pow(y2,2);
    double dp=pow(d,2);
    double mp=pow(m,2);
    double h=b-y2;
    //printf("(h)=%lf\n",h);
    double hp=pow(h,2);
    double k=((-hp)+dp-xp);
    double z=(2*x2)-(2*m);
    double w=(-1-mp);
    
    //printf("(K)=%lf\n",k);
    //printf("(Z)=%lf\n",z);
    //printf("(W)=%lf\n",w);
	printf("(Y1)=%lf (X1)=%lf ",y1,x1);
    printf("(Y2)=%lf (X2)=%lf\n",y2,x2);
    double del=(pow(z,2))-(4*w*k);
    if(del<0)
    {
        printf("out of range\n");    
        return ;
    }
    del=sqrt(del);
    double xx1=((-z)+(del))/(2*w);
    double xx2=((-z)-(del))/(2*w);
    //printf("(X1)=%lf\n",xx1);
    //printf("(X2)=%lf\n",xx2);
    
    double yy1=(m*xx1)+b;
    double yy2=(m*xx2)+b;
    printf("(Y1)=%lf (X1)=%lf\n",yy1,xx1);
    printf("(Y2)=%lf (X2)=%lf\n",yy2,xx2);

} 
void DrawLimb(XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2,char a[])
{
	
	if (!g_UserGenerator.GetSkeletonCap().IsTracking(player))
	{
		printf("not tracked!\n");
		return;
	}

	XnSkeletonJointPosition joint1, joint2;
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint1, joint1);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint2, joint2);

	if (joint1.fConfidence < 0.5 || joint2.fConfidence < 0.5)
	{
		return;
	}

	XnPoint3D pt[2];
	pt[0] = joint1.position;
	pt[1] = joint2.position;

	g_DepthGenerator.ConvertRealWorldToProjective(2, pt, pt);
	glVertex3i(pt[0].X, pt[0].Y, 0);
	glVertex3i(pt[1].X, pt[1].Y, 0);
	if(a[0]=='R')
	{
		glVertex3i(pt[1].X, pt[1].Y, 0);
	    glVertex3i(pt[1].X, pt[1].Y-20, 0);
	}
	
	if(a[0]=='L')
	{

		glColor3f(0,1,1);
		
		glColor3f(1,1,1);
	}
	
}
void text(int i)
{
	//glutCreateFont( "Times-Bold 12" ); GLUT_BITMAP_TIMES_ROMAN_24

	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,i);
}
void inpos(int* x,int* y,int wflag,XnPoint3D pt )
{
	//int x=*x1;
	//int y=*y1;
	if( state == 1  )
	{
		if(pt.X>=(*x)-10 && pt.X<=(*x)+10 && pt.Y>=(*y)-10 && pt.Y<=(*y)+10 || wflag==1)
		{
			glRasterPos2f(pt.X,pt.Y);
			(*x)=(int)pt.X;
			(*y)=(int)pt.Y;
			//printf(" \n(X) = %d (Y) = %d HX = %lf HY = %lf",x,y,pt[1].X,pt[1].Y);
			wflag=1;
		}
		else
		{
			glRasterPos2f((*x),(*y));
		}
	}
	else if( state == 0)
	{
		glRasterPos2f((*x),(*y));
		//printf(" \n(X) = %d (Y) = %d ",x,y);
		wflag=2;
	}
	//*x1=x;
	//*y1=y;
}
void chars(int xkey,int ykey,const char* Filenam,int i,XnPoint3D pt[] )
{
	int g,k,v;
	
	static int f,f2,f3;
	static int	pz,pz2;
	static int pi,count;
	g=20;
	glColor3f(0.5,0.5,0.5);
	
	v=50;
	int d1=distance(pt[2].X,pt[7].X,pt[2].Y,pt[7].Y,pt[2].Z,pt[7].Z,3);
	int d2=distance(pt[7].X,pt[3].X,pt[7].Y,pt[3].Y,pt[7].Z,pt[3].Z,3);
	int D=d1+d2;
	if( pt[3].X>=xkey && pt[3].X<=xkey+v && pt[3].Y>=ykey && pt[3].Y<=ykey+v || (pt[3].X>=xkey && pt[3].X<=xkey+27 && pt[3].Y>=ykey && pt[3].Y<=ykey+150 && i == 28)) //pt[1].X>=xkey && pt[1].X<=xkey+40 && pt[1].Y>=ykey && pt[1].Y<=ykey+40 ||
	{
			
			
			
			
			
	
			if( pt[3].X>=xkey-g && pt[3].X<=xkey+v+g && pt[3].Y>=ykey-g && pt[3].Y<=ykey+v+g || (pt[3].X>=xkey-g && pt[3].X<=xkey+27+g && pt[3].Y>=ykey-g && pt[3].Y<=ykey+150+g && i == 28) )
			{
             if(((int)pt[5].Z-(int)pt[3].Z>=350) && ((int)pt[5].Z-(int)pt[3].Z<=400) )
			 {
				if(f==0 )
				{
					if(i < 27 && charnum < 11  )
					{
						
						f2=1;
					}
					else if(i==27 && charnum > 0 )
					{
						f2=2;
					}
					else if( i==28 )
					{
						f2=3;
					}
					
				}
				

			 }
			 
			}
			if(((int)pt[5].Z-(int)pt[3].Z<=350) )
			{
				
				if(f2 == 1 )
				{
					    glColor3f(0.5,0,1);
						writing[charnum]=i;
						charnum++;
				}
				if( f2 ==2)
				{
					charnum--;
				}
				if( f2 ==3)
				{
					charnum=0;
				}
				f2=0;
			}
			
			glBindTexture(GL_TEXTURE_2D, texture[i]);
			glBegin(GL_POLYGON);
			
			if(i < 28)
			{
				glTexCoord2f(0.0f, 0.0f); glVertex2f(xkey-g, ykey-g);
				glTexCoord2f(1.0f, 0.0f); glVertex2f(xkey+50+g, ykey-g);
				glTexCoord2f(1.0f, 1.0f); glVertex2f(xkey+50+g, ykey+50+g);
				glTexCoord2f(0.0f, 1.0f); glVertex2f(xkey-g,ykey+50+g);
			}
			else if(i == 28)
			{
				glTexCoord2f(0.0f, 0.0f); glVertex2f(xkey-g, ykey-g);
				glTexCoord2f(1.0f, 0.0f); glVertex2f(xkey+27+g, ykey-g);
				glTexCoord2f(1.0f, 1.0f); glVertex2f(xkey+27+g, ykey+150+g);
				glTexCoord2f(0.0f, 1.0f); glVertex2f(xkey-g,ykey+150+g);
			}
			glEnd();
	}
	if( pt[1].X>=xkey && pt[1].X<=xkey+v && pt[1].Y>=ykey && pt[1].Y<=ykey+v || (pt[1].X>=xkey && pt[1].X<=xkey+27 && pt[1].Y>=ykey && pt[1].Y<=ykey+150 && i == 28))
	{
		    
			
			if( pt[1].X>=xkey-g && pt[1].X<=xkey+v+g && pt[1].Y>=ykey-g && pt[1].Y<=ykey+v+g ||( pt[1].X>=xkey-g && pt[1].X<=xkey+27+g && pt[1].Y>=ykey-g && pt[1].Y<=ykey+150+g && i == 28) )
			{
             if(((int)pt[4].Z-(int)pt[1].Z>=350) && ((int)pt[4].Z-(int)pt[1].Z<=400))
			 {
				if(f==0 )
				{
					if(i < 27 && charnum < 11 )
					{
						
						f3=1;
					}
					else if(i==27 && charnum > 0 )
					{
						f3=2;
					}
					else if( i==28 )
					{
						f3=3;
					}
					
				}
				

			 }
			 
			}
			if(((int)pt[4].Z-(int)pt[1].Z<=350) )
			{
				
				if(f3 == 1 )
				{
					    glColor3f(0.5,0,1);
						writing[charnum]=i;
						charnum++;
				}
				if( f3 ==2)
				{
					charnum--;
				}
				if( f3 ==3)
				{
					charnum=0;
				}
				f3=0;
			}
		
			glBindTexture(GL_TEXTURE_2D, texture[i]);
			glBegin(GL_POLYGON);
			if(i < 28)
			{
				glTexCoord2f(0.0f, 0.0f); glVertex2f(xkey-g, ykey-g);
				glTexCoord2f(1.0f, 0.0f); glVertex2f(xkey+50+g, ykey-g);
				glTexCoord2f(1.0f, 1.0f); glVertex2f(xkey+50+g, ykey+50+g);
				glTexCoord2f(0.0f, 1.0f); glVertex2f(xkey-g,ykey+50+g);
			}
			else if(i == 28)
			{
				printf("\n i = %d",i);
				glTexCoord2f(0.0f, 0.0f); glVertex2f(xkey-g, ykey-g);
				glTexCoord2f(1.0f, 0.0f); glVertex2f(xkey+27+g, ykey-g);
				glTexCoord2f(1.0f, 1.0f); glVertex2f(xkey+27+g, ykey+150+g);
				glTexCoord2f(0.0f, 1.0f); glVertex2f(xkey-g,ykey+150+g);
			}
			glEnd();
	}
	for(k=0; k<charnum; k++)
	{
		chwrite[k]=(char)(writing[k]+140);
	}
	if(charnum>0)
	{
		for(k=0; k<5; k++)
		{
			if (strncmp (names[k],chwrite,charnum) == 0)
			{
				printf("\nThe match found");
				printf("\n%s",names[k]);
			}
		}
	}


}
void chars2(int xkey,int ykey,const char* Filenam,int i,XnPoint3D pt[] )
{
	int g,v;
	
	static int f,f2,f3;
	static int	pz,pz2;
	static int pi,count;
	g=20;
	glColor3f(0.5,0.5,0.5);
	
	v=50;
	
	if( pt[3].X>=xkey && pt[3].X<=xkey+v && pt[3].Y>=ykey && pt[3].Y<=ykey+v || (pt[3].X>=xkey && pt[3].X<=xkey+27 && pt[3].Y>=ykey && pt[3].Y<=ykey+150 && i == 28)) //pt[1].X>=xkey && pt[1].X<=xkey+40 && pt[1].Y>=ykey && pt[1].Y<=ykey+40 ||
	{
						
			if(pt[3].Z <pt[1].Z)
			{
			  if(pt[1].X<pt[4].X-50)
			  {
				    if(i < 27 && charnum < 11 )
					{
						
						f3=1;
					}
					else if(i==27 && charnum > 0 )
					{
						f3=2;
					}
					else if( i==28 )
					{
						f3=3;
					}
			  }
			  else if(pt[1].X>pt[4].X-20)
			  {
				  if(f3 == 1 )
				  {
					    glColor3f(0.5,0,1);
						writing[charnum]=i;
						charnum++;
				  }
				  if( f3 ==2)
				  {
					charnum--;
				  }
				  if( f3 ==3)
				  {
					charnum=0;
				  }
				  f3=0;
			  }
			  glBindTexture(GL_TEXTURE_2D, texture[i]);
			  glBegin(GL_POLYGON);
			
			  if(i < 28)
			  {
				glTexCoord2f(0.0f, 0.0f); glVertex2f(xkey-g, ykey-g);
				glTexCoord2f(1.0f, 0.0f); glVertex2f(xkey+50+g, ykey-g);
				glTexCoord2f(1.0f, 1.0f); glVertex2f(xkey+50+g, ykey+50+g);
				glTexCoord2f(0.0f, 1.0f); glVertex2f(xkey-g,ykey+50+g);
			  }
			  else if(i == 28)
			  {
				glTexCoord2f(0.0f, 0.0f); glVertex2f(xkey-g, ykey-g);
				glTexCoord2f(1.0f, 0.0f); glVertex2f(xkey+27+g, ykey-g);
				glTexCoord2f(1.0f, 1.0f); glVertex2f(xkey+27+g, ykey+150+g);
				glTexCoord2f(0.0f, 1.0f); glVertex2f(xkey-g,ykey+150+g);
			  }
			  glEnd();
			}
	}
	if( pt[1].X>=xkey && pt[1].X<=xkey+v && pt[1].Y>=ykey && pt[1].Y<=ykey+v || (pt[1].X>=xkey && pt[1].X<=xkey+27 && pt[1].Y>=ykey && pt[1].Y<=ykey+150 && i == 28))
	{
		    
			
			
			if(pt[1].Z <pt[3].Z)
			{
			  if(pt[3].X>pt[2].X+50)
			  {
				    if(i < 27 && charnum < 11 )
					{
						
						f2=1;
					}
					else if(i==27 && charnum > 0 )
					{
						f2=2;
					}
					else if( i==28 )
					{
						f2=3;
					}
			  }
			  else if(pt[3].X<pt[2].X+20)
			  {
				  if(f2 == 1 )
				  {
					    glColor3f(0.5,0,1);
						writing[charnum]=i;
						charnum++;
				  }
				  if( f2 ==2)
				  {
					charnum--;
				  }
				  if( f2 ==3)
				  {
					charnum=0;
				  }
				  f2=0;
			  }	
			  glBindTexture(GL_TEXTURE_2D, texture[i]);
			  glBegin(GL_POLYGON);
			  if(i < 28)
			  {
				glTexCoord2f(0.0f, 0.0f); glVertex2f(xkey-g, ykey-g);
				glTexCoord2f(1.0f, 0.0f); glVertex2f(xkey+50+g, ykey-g);
				glTexCoord2f(1.0f, 1.0f); glVertex2f(xkey+50+g, ykey+50+g);
				glTexCoord2f(0.0f, 1.0f); glVertex2f(xkey-g,ykey+50+g);
			  }
			  else if(i == 28)
			  {
				printf("\n i = %d",i);
				glTexCoord2f(0.0f, 0.0f); glVertex2f(xkey-g, ykey-g);
				glTexCoord2f(1.0f, 0.0f); glVertex2f(xkey+27+g, ykey-g);
				glTexCoord2f(1.0f, 1.0f); glVertex2f(xkey+27+g, ykey+150+g);
				glTexCoord2f(0.0f, 1.0f); glVertex2f(xkey-g,ykey+150+g);
			  }
			  glEnd();
			}
	}
	
	

}
void DrawLimb1(XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2,XnSkeletonJoint eJoint3,XnSkeletonJoint eJoint4,XnSkeletonJoint eJoint5,XnSkeletonJoint eJoint6,XnSkeletonJoint eJoint7,XnSkeletonJoint eJoint8)
{

	
	static int red=1;
	static int green,blue;
	int gflag=0;
	int static wflag=0;
	static int x=200;
    static int y=200;
	static int xpos[26];
	static int ypos[26];
	static int pf=0;
	static int num[26];
	static int i;
	static int count=0;
	static int dc;
	static int j; 
	static int select;
	static int chfont;
	static int kx;
	static int ky;
	static int flag;
	static int switchflag;
	static int tflag;
	if (!g_UserGenerator.GetSkeletonCap().IsTracking(player))
	{
		printf("not tracked!\n");
		return;
	}
	//printf("\nj = %d",j);
	if(j==0)
	{
		for(i=0; i<26; i++)
		{
			xpos[i]=200;
			ypos[i]=200;
			num[i]=65;
			
			
		}
		
		j++;
	}
	XnSkeletonJointPosition joint1, joint2,joint3,joint4,joint5,joint6,joint7,joint8;
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint1, joint1);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint2, joint2);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint3, joint3);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint4, joint4);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint5, joint5);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint6, joint6);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint7, joint7);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint8, joint8);

	if (joint1.fConfidence < 0.5 || joint2.fConfidence < 0.5)
	{
		return;
	}

	XnPoint3D pt[8];
	
	pt[0] = joint1.position;
	pt[1] = joint2.position;
	pt[2] = joint3.position;
	pt[3] = joint4.position;
	pt[4] = joint5.position;
	pt[5] = joint6.position;
	pt[6] = joint7.position;
	pt[7] = joint8.position;
	g_DepthGenerator.ConvertRealWorldToProjective(8, pt, pt);
	
    
	
	
	if(tflag == 0)
	{
			kx=(int)pt[5].X;
			ky=(int)pt[5].Y;
		
	}

	

	//Change color to purple
	//Stop drawing our polygon
	//*/
    // Front Face
	 glColor3f(0.5,0.5,0.5);
	
	
	glEnable(GL_TEXTURE_2D);	
	
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		//int g=0;
		if(pt[1].X>=100 && pt[1].X<=200 && pt[1].Y>100 && pt[1].Y<=200)
		{
			//g=50;
		}
		//printf(" X = %d Y = %d",(int)pt[5].X);
	    glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(kx-256.0f, ky-128.0f-50);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(kx+256.0f, ky-128.0f-50);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(kx+256.0f, ky+128.0f-50);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(kx-256.0f, ky+128.0f-50);
		glEnd();
		int xkey=kx-256.0;
		int ykey=ky-128.0-50+75;
		
		int gl;
		int g=0;
		char img[]={"Data/A.bmp"};
		for(gl=0; gl<3; gl++)
		{
		  for(int l=0; l<9; l++)
		  {
			if((l+(9*gl)+1)==27)
			{
			
				chars2(xkey+(52*l),ykey+(52*gl),"Data/Del.bmp",l+(9*gl)+1,pt );
			}
			else
			{
			 
			  chars2(xkey+(52*l),ykey+(52*gl),img,l+(9*gl)+1,pt );
			
			  img[5]++;
			
			}
			
		  }
		}
		
	
		chars2(xkey+485,ykey,"Data/clear1.bmp",28,pt);
		tflag=1;
		
		
		g=44;
		glColor3f(0.5,0.5,0.5);
		for(int l=0; l<charnum; l++)
		{
			
			glBindTexture(GL_TEXTURE_2D, texture[writing[l]]);
			glBegin(GL_POLYGON);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(xkey+17+(g*l), ykey-25);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(xkey+54+(g*l),ykey-25); 
			glTexCoord2f(1.0f, 0.0f); glVertex2f(xkey+54+(g*l), ykey-60);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(xkey+17+(g*l), ykey-60);
			glEnd();
		
		}	
		
		
		glBegin(GL_POLYGON);//Start drawing a polygon
	    for(int i=0; i<=360; i++)
		{
		 glColor3f(1,0,0);
         int x=pt[1].X+(10* cos(i*3.14/180));
		 glColor3f(1,0,1);
		 int y=pt[1].Y+(10* sin(i*3.14/180));
		 glVertex2f(x,y);//last vertex
		}
		//Change color to purple
		glEnd();//Stop drawing our polygon
		glBegin(GL_POLYGON);//Start drawing a polygon
	    for(int i=0; i<=360; i++)
		{
		 glColor3f(1,0,0);
         int x=pt[3].X+(10* cos(i*3.14/180));
		 glColor3f(1,0,1);
		 int y=pt[3].Y+(10* sin(i*3.14/180));
		 glVertex2f(x,y);//last vertex
		}
		//Change color to purple
		glEnd();//Stop drawing our polygon
		
		for(int l=1; l<=charnum; l++)
		{
		chwrite[l-1]=(char)(writing[l-1]+96);
		
		}
		
		printf("\n%s",chwrite);
		gl=0;
		if(charnum>0)
		{
		for(int l=1; l<=5; l++)
		{
			if (strncmp (names[l-1],chwrite,charnum) == 0)
			{
				
				strcpy(found[gl],names[l-1]);
				//printf("\n%s",found[l-1]);

				gl++;
			}
		}
		}
		for(int l=0; l<gl; l++)
		{
			printf("\nThe match found");
			printf("\n%s",found[l]);
		}

		/*if(charnum == 1)
		{
	    chwrite[0]=(char)(writing[0]+96);
		printf("\n%s",chwrite);
		}*/
}
int maxValue(int array[])
{
	 int j=0; 
     int length = sizeof(array);  // establish size of array
     int max = array[0];       // start with max = first element

     for(int i = 1; i<length; i++)
     {
          if(array[i] > max)
		  {
                max = array[i];
				 j=i;
		  }
			
     }
     return j;                // return highest value in array
}
int minValue(int array[])
{
	 int j=0; 
     int length = sizeof(array);  // establish size of array
     int min = array[0];       // start with max = first element

     for(int i = 1; i<length; i++)
     {
          if(array[i] < min)
		  {
                min = array[i];
				 j=i;
		  }
		       
     }
     return j;                // return highest value in array
}
void  Sendposition1(XnUserID Player, int  id,XnSkeletonJoint Part ,XnSkeletonJoint Part1,XnSkeletonJoint Part2)
{
	
	const XnDepthPixel* pDepthMap = g_DepthGenerator.GetDepthMap();


 if  (!g_UserGenerator.GetSkeletonCap().IsTracking(Player))
 {
  printf("not tracked! \n");
  return ;
 }
 XnSkeletonJointPosition joint,joint1,joint2;
 g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(Player, Part,  joint);
 g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(Player, Part1, joint1);
 g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(Player, Part2, joint2);
 XnPoint3D pt [3];
 if (joint1.fConfidence < 0.5 )
	{
		return;
	}

 pt [0] = joint.position;
 pt [1]= joint1.position;
 pt [2]= joint2.position;
 //printf("\n%d\n",Part2);
 g_DepthGenerator.ConvertProjectiveToRealWorld(3,pt,pt);
 //printf("\n Real X = %d Y = %d Z = %d ",(int)pt[2].X,(int)pt[2].Y,(int)pt[2].Z) ; 
 int d1=distance(pt[0].X,pt[1].X,pt[0].Y,pt[1].Y,pt[0].Z,pt[1].Z,3);
 int d2=distance(pt[1].X,pt[2].X,pt[1].Y,pt[2].Y,pt[1].Z,pt[2].Z,3);
 int D=d1+d2;
 int dshtoh=distance(pt[0].X,pt[2].X,pt[0].Y,pt[2].Y,pt[0].Z,pt[2].Z,3);
 //printf(" \n D         = %d ",D);
 //printf(" \n D sh to h = %d ",dshtoh);

 
 if( Part2 == 9  )
 {
	 if(dshtoh <=D && dshtoh >= (D-300))
	 state=1;
	 else
	 state=0;
 }
 if( Part2 == 15)
 {
	 if(dshtoh <=D && dshtoh >= (D-300))
	 stater=1;
	 else
	 stater=0;
 }
  
 //int d1=ditance(
 
 
}
 void  Sendposition2(XnUserID Player, int  id,XnSkeletonJoint Part ,XnSkeletonJoint Part1,XnSkeletonJoint Part2,XnSkeletonJoint Part3,XnSkeletonJoint Part4,XnSkeletonJoint Part5,XnSkeletonJoint Part6)
{
	
 int static i=0,j,k;
 int static d[20],dt=0;
 int static dp,dn,dt2,df;
 int static flag1,flag2,flag3;
 int static mousef;
 int static reflag;
 int static hipflag;
 int static newx,prex,deltax;
 int static newy,prey,deltay;
 int static secprex,secprey;
 int static refpointx,refpointy;
 int static flagp;
 int static dflag;
 int static rflag=1;
 int static lflag=1;
 int static uflag=1;
 int static downflag=1;

 const XnDepthPixel* pDepthMap = g_DepthGenerator.GetDepthMap();
 if  (!g_UserGenerator.GetSkeletonCap().IsTracking(Player))
 {
  printf("not tracked! \n");
  return ;
 }
 XnSkeletonJointPosition joint,joint1,joint2,joint3,joint4,joint5,joint6;
 g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(Player, Part,  joint);
 g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(Player, Part1, joint1);
 g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(Player, Part2, joint2);
 g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(Player, Part3, joint3);
 g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(Player, Part3, joint4);
 g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(Player, Part3, joint5);
 g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(Player, Part3, joint6);

 XnPoint3D pt [6];
 if (joint1.fConfidence < 0.5 )
	{
		return;
	}

 pt [0] = joint.position;
 pt [1]= joint1.position;
 pt [2]= joint2.position;
 pt [3]= joint3.position;
 //pt [4]= joint4.position;
 pt [4]= joint5.position;
 pt [5]=joint6.position;
 //printf("\n(X) EL = %d   EL = %d",(int)pt[4].X,(int)pt[4].Y);
 g_DepthGenerator.ConvertProjectiveToRealWorld(4,pt,pt);
 dp=dn;
 //printf("\n Real X = %d Y = %d Z = %d ",(int)pt[3].X,(int)pt[3].Y,(int)pt[3].Z) ; 
 int d1=distance(pt[0].X,pt[1].X,pt[0].Y,pt[1].Y,0,0,2);
 dn=d1;
 dt2=dn-dp;
 df=d1-300;
 
 




 if( state ==0 && stater == 0)
 {
	adwsflag=0;
	hipflag=0;
	shiftkflag=0;
	//rflag=downflag=lflag=uflag=1;
	keybd_event(VK_LSHIFT,0xaa,KEYEVENTF_KEYUP , 0);
	keybd_event(VkKeyScan('S'),0x9f, KEYEVENTF_KEYUP,0);
    keybd_event(VkKeyScan('W'),0x91, KEYEVENTF_KEYUP,0);
	keybd_event(VkKeyScan('D'),0xa0, KEYEVENTF_KEYUP,0);
	keybd_event(VkKeyScan('A'),0x9e, KEYEVENTF_KEYUP,0);
 }
 if((pt[3].Y<=pt[0].Y) && (pt[2].Y<=pt[1].Y))
 {
    hipflag=1;

	
 }
 if(dt2>50)
 {
	 //printf("\npositive direction");
	 if(reflag!=2)
	 flag1=1;

 }
 else if(dt2<-50)
 {
	 //printf("\nnegative direction");
	 if(reflag!=1)
	 flag1=2;
 }
 else;
 //printf("\n(X) EL = %d   EL = %d",(int)pt[4].X,(int)pt[4].Y);
 if((pt[3].Y<=pt[0].Y) && adwsflag!=1)
 {
	 //printf("\n(Y)  = %d (X) = %d",(int)pt[1].Y-(int)pt[5].Y,(int)pt[1].X-(int)pt[5].X);
	 shiftkflag=1;
	 keybd_event(VK_LSHIFT,0xaa,KEYEVENTF_KEYUP, 0);
	 keybd_event(VkKeyScan('S'),0x9f, KEYEVENTF_KEYUP,0);
	 keybd_event(VkKeyScan('W'),0x91, KEYEVENTF_KEYUP,0);
	 keybd_event(VkKeyScan('D'),0xa0, KEYEVENTF_KEYUP,0);
	 keybd_event(VkKeyScan('A'),0x9e, KEYEVENTF_KEYUP,0);
	 if(((pt[1].Y-pt[5].Y)>700 || (pt[2].Y<=pt[1].Y)) )
	 {
		 //printf("\nright hand to the ""down"" direction");	
		 keybd_event(VK_LSHIFT,0xaa,0 , 0);
		 keybd_event(VkKeyScan('S'),0x9f,0 , 0);
		 
	 }
	 if((pt[1].Y-pt[5].Y)<=-500 )
	 {
		 //printf("\nright hand to the ""up"" direction");
		 keybd_event(VK_LSHIFT,0xaa,0 , 0);
		 keybd_event(VkKeyScan('W'),0x91,0 , 0);
		 
	 }
	 if((pt[1].X-pt[5].X )<=-1500  )
	 {
		 //printf("\nright hand to the ""left"" direction ");
		 keybd_event(VK_LSHIFT,0xaa,0 , 0);
		 keybd_event(VkKeyScan('A'),0x9e,0 , 0);
		 
	 }
	 if((pt[1].X-pt[5].X)>40  )
	 {
		 //printf("\nright hand to the ""right"" direction "); 
		 keybd_event(VK_LSHIFT,0xaa,0 , 0);
		 keybd_event(VkKeyScan('D'),0xa0,0 , 0);
		 
	 }
	 
 }
 if(pt[2].Y<=pt[1].Y && shiftkflag!=1 )
 {
	 adwsflag=1;
	 //printf("\n(Y)  = %d (X) = %d",(int)pt[0].Y-(int)pt[4].Y,(int)pt[0].X-(int)pt[4].X);
	 //keybd_event(VK_LSHIFT,0xaa,KEYEVENTF_KEYUP , 0);
	 keybd_event(VkKeyScan('S'),0x9f, KEYEVENTF_KEYUP,0);
	 keybd_event(VkKeyScan('W'),0x91, KEYEVENTF_KEYUP,0);
	 keybd_event(VkKeyScan('D'),0xa0, KEYEVENTF_KEYUP,0);
	 keybd_event(VkKeyScan('A'),0x9e, KEYEVENTF_KEYUP,0);
	 if(((pt[0].Y-pt[4].Y)>700 || (pt[3].Y<=pt[0].Y)) )
	 {
		// printf("\nright hand to the ""down"" direction");	
		 //keybd_event(VK_LSHIFT,0xaa,0 , 0);
		 keybd_event(VkKeyScan('S'),0x9f,0 , 0);
		 //rflag=uflag=lflag=0;
	 }
	 if((pt[0].Y-pt[4].Y)<=-500 )
	 {
		 //printf("\nright hand to the ""up"" direction");
		 //keybd_event(VK_LSHIFT,0xaa,0 , 0);
		 keybd_event(VkKeyScan('W'),0x91,0 , 0);
		 //rflag=downflag=lflag=0;
	 }
	 if((pt[0].X-pt[4].X )<=-500  )
	 {
		 //printf("\nright hand to the ""left"" direction ");
		 //keybd_event(VK_LSHIFT,0xaa,0 , 0);
		 keybd_event(VkKeyScan('A'),0x9e,0 , 0);
		 //rflag=uflag=downflag=0;
	 }
	 if((pt[0].X-pt[4].X)>400  )
	 {
		 //printf("\nright hand to the ""right"" direction "); 
		 //keybd_event(VK_LSHIFT,0xaa,0 , 0);
		 keybd_event(VkKeyScan('D'),0xa0,0 , 0);
		 //downflag=uflag=lflag=0;
	 }
	 
	 
 }
 if(hipflag!=1 && adwsflag != 1 && shiftkflag!=1 )
 {
	 
	
	if(state == 1 && stater == 1)
	{
		if(flag1==1)
		{
			reflag=1; 
			if(flag2==1 && reflag==1)
			{
				mouse_event(MOUSEEVENTF_WHEEL,0,0,120,0);
				flag2=0;
			}
	 
	 
		flag2=1;
		flag1=0;
	 	
	   }
 
	   if( flag1==2 )
	   {

			reflag=2;
			if(flag3==1 && reflag==2)
			{
				mouse_event(MOUSEEVENTF_WHEEL,0,0,-120,0);
				flag3=0;
			}
	  
			flag3=1;
			flag1=0;
	 	
	  }
   }
   else if(state == 1 && stater ==  5 )
   {
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			mousef=1;
	
			printf("\nyeesdown");
   }
   else if( state ==0 && stater == 0  )
   {
			if(mousef==1)
			{
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				printf("\nyessup");
				mousef=0;
			}
			reflag=0;
   }
   else;
 }
 

 
}
void  Sendposition(XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2,XnSkeletonJoint eJoint3,XnSkeletonJoint eJoint4,XnSkeletonJoint eJoint5,XnSkeletonJoint eJoint6,XnSkeletonJoint eJoint7,XnSkeletonJoint eJoint8)
{
	int static i,j;
	int static f3,f2;
	const XnDepthPixel* pDepthMap = g_DepthGenerator.GetDepthMap();


    if(!g_UserGenerator.GetSkeletonCap().IsTracking(player))
    {
		printf("not tracked! \n");
		return ;
    }
	XnSkeletonJointPosition joint1, joint2,joint3,joint4,joint5,joint6,joint7,joint8;
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint1, joint1);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint2, joint2);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint3, joint3);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint4, joint4);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint5, joint5);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint6, joint6);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint7, joint7);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint8, joint8);

	if (joint1.fConfidence < 0.5 || joint2.fConfidence < 0.5)
	{
		return;
	}

	XnPoint3D pt[8];
	
	pt[0] = joint1.position;
	pt[1] = joint2.position;
	pt[2] = joint3.position;
	pt[3] = joint4.position;
	pt[4] = joint5.position;
	pt[5] = joint6.position;
	pt[6] = joint7.position;
	pt[7] = joint8.position;
	g_DepthGenerator.ConvertRealWorldToProjective(8, pt, pt);
    

	

	

//if(1)//pt[3].Z-50 <pt[1].Z)
//{
			  ProcessNewHandPos((int)pt[3].X,(int)pt[3].Y); // the mosue should be the right hand 	
			  if(pt[1].X<pt[4].X-50 && f3 == 0)
			  {
				    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
					f3=1;
			  }
			  if(pt[1].X>pt[4].X-20)
			  {
				  if(f3 == 1 )
				  {
					 mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
					 mouse_event(MOUSEEVENTF_LEFTUP,  0, 0,0, 0);  
					 
				  }
				  f3=0;
			  }
			  
//}
/*if(0)
{
			  ProcessNewHandPos(pt[1].X,pt[1].Y); // the mosue should be the right hand 	
			  if(pt[3].X<pt[2].X-50)
			  {
				    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
					f2=1;
			  }
			  else if(pt[3].X>pt[2].X-20)
			  {
				  if(f2 == 1 )
				  {
					 mouse_event(MOUSEEVENTF_LEFTUP,  0, 0,0, 0);  
				  }
				  f2=0;
			  }
			  
}*/
 
 

}
int round(double m)
{
	if(m-(int)m>=0.5)
		return(ceil(m));
	else if(m-(int)m<0.5)
		return(floor(m));
}
void ProcessNewHandPos(int NewX, int NewY)
{
	POINT CurrentMousePos; 
	GetCursorPos(&CurrentMousePos);

	static double SubPixelCarryoverX = 0, SubPixelCarryoverY=0 ;
	static double PrevX = (double)NewX, PrevY = (double)NewY; 
	double dx = (double)NewX - PrevX;
	double dy = (double)NewY - PrevY;
	double v;
	
	v = (1)*sqrt(dx*dx + dy*dy );
	v=round(v);
	v = 1.0*v + 0.09*v*v; //acceleration here.
	//printf("v:%lf\n", v);

	double NewMousePosX =round( CurrentMousePos.x + SubPixelCarryoverX + dx*v);
	double NewMousePosY = round(CurrentMousePos.y + SubPixelCarryoverY + dy*v);
	//double NewMousePosX = CurrentMousePos.x + SubPixelCarryoverX + (dx*v)+(dx*v)/2;
	//double NewMousePosY = CurrentMousePos.y + SubPixelCarryoverY + (dy*v)+(dy*v)/2;

	SetCursorPos((int)NewMousePosX, (int)NewMousePosY);
	
	SubPixelCarryoverX = ( dx*v - (int)dx*v );
	SubPixelCarryoverY = ( dy*v - (int)dy*v );
	
	PrevX = NewX;
	PrevY = NewY;
}

void ProcessNewHandPos2(int NewX, int NewY)
{
	POINT CurrentMousePos; 
	GetCursorPos(&CurrentMousePos);
	static int flag1;
	static double SubPixelCarryoverX = 0, SubPixelCarryoverY=0 ;
	static int PrevX = NewX, PrevY = NewY; 
	time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ( "\nCurrent local time and date: %s", asctime (timeinfo) );
	static int newt=(*timeinfo).tm_sec;
	static int pret;
	
	//static time_t pret;
	//static time_t newt=time (NULL);

	float dx =float ( NewX - PrevX);
	float dy =float (NewY - PrevY);
	double v;
	static double pMousePosX[2];
	static double pMousePosY[2];
	float x_speed;
	float y_speed;
	//printf("\n dt = %ld",newt);
	
	//float dt = (int)difftime (newt,pret);
	//dt=dt/1000000.0;
	printf("\n x = %d y = %d",NewX,NewY );
	printf("\n dx = %d dy = %d",(int)dx,(int)dy );
	int dt=newt-pret;
	x_speed = dx/4.0;
	y_speed = dy/4.0;
	
	printf("\n dt = %lf",dt);
	printf("\nxv = %lf yv = %lf",x_speed,y_speed );
	//double NewMousePosX = round(CurrentMousePos.x + SubPixelCarryoverX + (dx*v));
	//double NewMousePosY = CurrentMousePos.y + SubPixelCarryoverY + (dy*v);
	int xpos = CurrentMousePos.x+(int)round(x_speed*sqrt(1680.0));
	if(xpos<0) xpos=0; if(xpos>1680) xpos=1680;
	int ypos = CurrentMousePos.y+(int)round(y_speed*sqrt(1050.0));
	if(ypos<0) ypos=0; if(ypos>1050.0) ypos=1050.0;
	//double NewMousePosX = CurrentMousePos.x + SubPixelCarryoverX + (dx*v)+(dx*v)/2;
	//double NewMousePosY = CurrentMousePos.y + SubPixelCarryoverY + (dy*v)+(dy*v)/2;
	/*double NewMousePosX = CurrentMousePos.x+(int)(v*sqrt(640.0));
	if(NewMousePosX<0) NewMousePosX=0; if(NewMousePosX>640) NewMousePosX=640;
	double NewMousePosY = CurrentMousePos.y+(int)(v*sqrt(480.0));
	if(NewMousePosY<0) NewMousePosY=0; if(NewMousePosY>480) NewMousePosY=480;*/
	
	SetCursorPos(xpos,ypos);
	
	
	//SubPixelCarryoverX = ( dx*v - (int)dx*v );
	//SubPixelCarryoverY = ( dy*v - (int)dy*v );
	//SubPixelCarryoverX=SubPixelCarryoverY =0;
	PrevX = NewX;
	PrevY = NewY;
	pret=newt;

}
void ProcessNewHandPos1(int NewX, int NewY)
{
	POINT CurrentMousePos; 
	GetCursorPos(&CurrentMousePos);
	static int flag1;
	static double SubPixelCarryoverX = 0, SubPixelCarryoverY=0 ;
	static double PrevX = (double)NewX, PrevY = (double)NewY; 
	int dx =int( (double)NewX - PrevX);
	int dy =int ((double)NewY - PrevY);
	double v;
	static double pMousePosX[2];
	static double pMousePosY[2];
	//v = sqrt(dx*dx + dy*dy );
	v=10;
	//v = 1.0*v + 0.09*v*v; //acceleration here.
	//printf("v:%lf\n", v);
	
	double NewMousePosX = CurrentMousePos.x + SubPixelCarryoverX + round(dx*v);
	double NewMousePosY = CurrentMousePos.y + SubPixelCarryoverY + round(dy*v);
	//double NewMousePosX = CurrentMousePos.x + SubPixelCarryoverX + (dx*v)+(dx*v)/2;
	//double NewMousePosY = CurrentMousePos.y + SubPixelCarryoverY + (dy*v)+(dy*v)/2;
	/*double NewMousePosX = CurrentMousePos.x+(int)(v*sqrt(640.0));
	if(NewMousePosX<0) NewMousePosX=0; if(NewMousePosX>640) NewMousePosX=640;
	double NewMousePosY = CurrentMousePos.y+(int)(v*sqrt(480.0));
	if(NewMousePosY<0) NewMousePosY=0; if(NewMousePosY>480) NewMousePosY=480;*/
	if(flag1 == 3)
	{
		SetCursorPos((int)((pMousePosX[0]+pMousePosX[1]+pMousePosX[2])/3), (int)((pMousePosY[0]+pMousePosY[1]+pMousePosX[2])/3));
		flag1=0;
	}
	
	//SubPixelCarryoverX = ( dx*v - (int)dx*v );
	//SubPixelCarryoverY = ( dy*v - (int)dy*v );
	//SubPixelCarryoverX=SubPixelCarryoverY =0;
	PrevX = NewX;
	PrevY = NewY;
	pMousePosX[flag1]=NewMousePosX;
	pMousePosY[flag1]=NewMousePosY;
	flag1++;

}
void DrawDepthMap(const xn::DepthMetaData& dmd, const xn::SceneMetaData& smd)
{
	//const XnLabel *a;
	static bool bInitialized = false;	
	static GLuint depthTexID;
	static unsigned char* pDepthTexBuf;
	static int texWidth, texHeight;

    float topLeftX;
	 float topLeftY;
	 float bottomRightY;
	 float bottomRightX;
	float texXpos;
	float texYpos;

	if(!bInitialized)
	{

		texWidth =  getClosestPowerOfTwo(dmd.XRes());
		texHeight = getClosestPowerOfTwo(dmd.YRes());

//		printf("Initializing depth texture: width = %d, height = %d\n", texWidth, texHeight);
		depthTexID = initTexture((void**)&pDepthTexBuf,texWidth, texHeight) ;

//		printf("Initialized depth texture: width = %d, height = %d\n", texWidth, texHeight);
		bInitialized = true;

		topLeftX = dmd.XRes();
		topLeftY = 0;
		bottomRightY = dmd.YRes();
		bottomRightX = 0;
		texXpos =(float)dmd.XRes()/texWidth;
		texYpos  =(float)dmd.YRes()/texHeight;

		memset(texcoords, 0, 8*sizeof(float));
		texcoords[0] = texXpos, texcoords[1] = texYpos, texcoords[2] = texXpos, texcoords[7] = texYpos;

	}
	unsigned int nValue = 0;
	unsigned int nHistValue = 0;
	unsigned int nIndex = 0;
	unsigned int nX = 0;
	unsigned int nY = 0;
	unsigned int nNumberOfPoints = 0;
	XnUInt16 g_nXRes = dmd.XRes();
	XnUInt16 g_nYRes = dmd.YRes();

	unsigned char* pDestImage = pDepthTexBuf;

	const XnDepthPixel* pDepth = dmd.Data();
	const XnLabel* pLabels = smd.Data();

	// Calculate the accumulative histogram
	memset(g_pDepthHist, 0, MAX_DEPTH*sizeof(float));
	for (nY=0; nY<g_nYRes; nY++)
	{
		for (nX=0; nX<g_nXRes; nX++)
		{
			nValue = *pDepth;

			if (nValue != 0)
			{
				g_pDepthHist[nValue]++;
				nNumberOfPoints++;
			}

			pDepth++;
		}
	}

	for (nIndex=1; nIndex<MAX_DEPTH; nIndex++)
	{
		g_pDepthHist[nIndex] += g_pDepthHist[nIndex-1];
	}
	if (nNumberOfPoints)
	{
		for (nIndex=1; nIndex<MAX_DEPTH; nIndex++)
		{
			g_pDepthHist[nIndex] = (unsigned int)(256 * (1.0f - (g_pDepthHist[nIndex] / nNumberOfPoints)));
		}
	}

	pDepth = dmd.Data();
	if (g_bDrawPixels)
	{
		XnUInt32 nIndex = 0;
		// Prepare the texture map
		for (nY=0; nY<g_nYRes; nY++)
		{
			for (nX=0; nX < g_nXRes; nX++, nIndex++)
			{

				pDestImage[0] = 0;
				pDestImage[1] = 0;
				pDestImage[2] = 0;
				if (g_bDrawBackground || *pLabels != 0)
				{
					nValue = *pDepth;
					XnLabel label = *pLabels;
					XnUInt32 nColorID = label % nColors;
					if (label == 0)
					{
						nColorID = nColors;
					}

					if (nValue != 0)
					{
						nHistValue = g_pDepthHist[nValue];

						pDestImage[0] = nHistValue * Colors[nColorID][0]; 
						pDestImage[1] = nHistValue * Colors[nColorID][1];
						pDestImage[2] = nHistValue * Colors[nColorID][2];
					}
				}

				pDepth++;
				pLabels++;
				pDestImage+=3;
			}

			pDestImage += (texWidth - g_nXRes) *3;
		}
	}
	else
	{
		xnOSMemSet(pDepthTexBuf, 0, 3*2*g_nXRes*g_nYRes);
	}

	glBindTexture(GL_TEXTURE_2D, depthTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pDepthTexBuf);

	// Display the OpenGL texture map
	glColor4f(0.75,0.75,0.75,1);

	glEnable(GL_TEXTURE_2D);
	DrawTexture(dmd.XRes(),dmd.YRes(),0,0);	
	glDisable(GL_TEXTURE_2D);

	char strLabel[50] = "";
	XnUserID aUsers[15];
	XnUInt16 nUsers = 15;
	g_UserGenerator.GetUsers(aUsers, nUsers);
	for (int i = 0; i < nUsers; ++i)
	{
		if (g_bPrintID)
		{
			XnPoint3D com;
			g_UserGenerator.GetCoM(aUsers[i], com);
			g_DepthGenerator.ConvertRealWorldToProjective(1, &com, &com);

			xnOSMemSet(strLabel, 0, sizeof(strLabel));
			if (!g_bPrintState)
			{
				// Tracking
				sprintf(strLabel, "%d", aUsers[i]);
				
			}
			else if (g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i]))
			{
				// Tracking
				sprintf(strLabel, "%d - Tracking", aUsers[i]);
				//g_bDrawPixels= false;
			}
			else if (g_UserGenerator.GetSkeletonCap().IsCalibrating(aUsers[i]))
			{
				// Calibrating
				sprintf(strLabel, "%d - Calibrating...", aUsers[i]);
				//g_bDrawPixels= true;
			}
			else
			{
				// Nothing
				sprintf(strLabel, "%d - Looking for pose", aUsers[i]);
				//g_bDrawPixels= true;
			}


			glColor4f(1-Colors[i%nColors][0], 1-Colors[i%nColors][1], 1-Colors[i%nColors][2], 1);

			glRasterPos2i(com.X, com.Y);
			glPrintString(GLUT_BITMAP_HELVETICA_18, strLabel);
		}
		if (g_bDrawSkeleton && g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i]))
		{
			//DrawLimb1(aUsers[i], XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND,XN_SKEL_RIGHT_SHOULDER,XN_SKEL_RIGHT_HAND, XN_SKEL_LEFT_SHOULDER,XN_SKEL_NECK, XN_SKEL_HEAD, XN_SKEL_RIGHT_ELBOW);
			Sendposition(aUsers[i], XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND,XN_SKEL_RIGHT_SHOULDER,XN_SKEL_RIGHT_HAND, XN_SKEL_LEFT_SHOULDER,XN_SKEL_NECK, XN_SKEL_HEAD, XN_SKEL_RIGHT_ELBOW);
		}

		if (g_bDrawSkeleton && g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i]))
		{
			glBegin(GL_LINES);
			glColor4f(1-Colors[aUsers[i]%nColors][0], 1-Colors[aUsers[i]%nColors][1], 1-Colors[aUsers[i]%nColors][2], 1);
			//Sendposition1(aUsers[i],i,XN_SKEL_LEFT_SHOULDER,XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND);
			//Sendposition1(aUsers[i],i,XN_SKEL_RIGHT_SHOULDER,XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND);
			//Sendposition2(aUsers[i],i,XN_SKEL_RIGHT_HAND, XN_SKEL_LEFT_HAND,XN_SKEL_LEFT_HIP,XN_SKEL_RIGHT_HIP,XN_SKEL_RIGHT_ELBOW,XN_SKEL_RIGHT_SHOULDER,XN_SKEL_LEFT_SHOULDER);
			//DrawLimb(aUsers[i], XN_SKEL_HEAD, XN_SKEL_NECK,"H");

			//DrawLimb(aUsers[i], XN_SKEL_NECK, XN_SKEL_LEFT_SHOULDER,"");
			//DrawLimb(aUsers[i], XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW,"");
			//DrawLimb(aUsers[i], XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND,"L");

			//DrawLimb(aUsers[i], XN_SKEL_NECK, XN_SKEL_RIGHT_SHOULDER,"");
			//DrawLimb(aUsers[i], XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW,"");
			//DrawLimb(aUsers[i], XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND,"");

			//g_UserGenerator.GetUserPixels( aUsers[i], scene  ) ; 
			//a=scene.Data();
			//Sendposition(aUsers[i],i,XN_SKEL_RIGHT_HAND,"RH");
			//Sendposition(aUsers[i],i,XN_SKEL_LEFT_HAND,"LH");
			//Sendposition1(aUsers[i],i,XN_SKEL_RIGHT_SHOULDER,XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND);
			//Sendposition1(aUsers[i],i,XN_SKEL_LEFT_SHOULDER,XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND);
			
			//DrawLimb(aUsers[i], XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO,"");
			//DrawLimb(aUsers[i], XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO,"");

			//DrawLimb(aUsers[i], XN_SKEL_TORSO, XN_SKEL_LEFT_HIP,"");
			//DrawLimb(aUsers[i], XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE,"");
			//DrawLimb(aUsers[i], XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT,"");

			//DrawLimb(aUsers[i], XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP,"");
			//DrawLimb(aUsers[i], XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE,"");
			//DrawLimb(aUsers[i], XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT,"");

			//DrawLimb(aUsers[i], XN_SKEL_LEFT_HIP, XN_SKEL_RIGHT_HIP,"");

			glEnd();
		}
	}
}



