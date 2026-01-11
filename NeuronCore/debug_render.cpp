#include "pch.h"
#include "math_utils.h"
#include "sphere_renderer.h"
#include "text_renderer.h"
#include "app.h"
#include "camera.h"
#include "debug_render.h"
#include "renderer.h"


#ifdef DEBUG_RENDER_ENABLED

static Sphere aSphere;

void RenderSphere(Vector3 const &_centre, float _radius, RGBAColour const &_col)
{
    if( Sphere::s_regenerateDisplayList )
    {
        aSphere.GenerateDisplayList();
    }

	glColor4ubv(_col.GetData());
	aSphere.Render(_centre, _radius);
}

void RenderVerticalCylinder(Vector3 const &_centreBase, Vector3 const &_verticalAxis,
							float _height, float _radius, RGBAColour const &_col)
{
	Vector3 axis1 = _verticalAxis;
	axis1.Normalise();
	Vector3 axis2;
	Vector3 axis3;

	if (axis1.x > 0.5)			axis2.Set(0, 1, 0);
	else						axis2.Set(1, 0, 0);

	axis3 = axis1 ^ axis2;
	axis2 = axis1 ^ axis3;

	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_TEXTURE_2D);
	glLineWidth(1.0);
	glColor3ubv(_col.GetData());

	int const numEdges = 16;

	// Base
	glBegin(GL_LINE_LOOP);
		for (int i = 0; i < numEdges; ++i)
		{
			Vector3 pos = _centreBase;
			float theta = M_PI * 2.0 * (float)i / (float)numEdges;
			pos += axis2 * sin(theta) * _radius;
			pos += axis3 * cos(theta) * _radius;
			glVertex3dv(pos.GetData());
		}
	glEnd();

	// Top
	glBegin(GL_LINE_LOOP);
		for (int i = 0; i < numEdges; ++i)
		{
			Vector3 pos = _centreBase + axis1 * _height;
			float theta = M_PI * 2.0 * (float)i / (float)numEdges;
			pos += axis2 * sin(theta) * _radius;
			pos += axis3 * cos(theta) * _radius;
			glVertex3dv(pos.GetData());
		}
	glEnd();

	// Middle
	glBegin(GL_LINE_LOOP);
		for (int i = 0; i < numEdges; ++i)
		{
			Vector3 pos = _centreBase;
			float theta = M_PI * 2.0 * (float)i / (float)numEdges;
			pos += axis2 * sin(theta) * _radius;
			pos += axis3 * cos(theta) * _radius;
			glVertex3dv(pos.GetData());
			pos += axis1 * _height;
			glVertex3dv(pos.GetData());
		}
	glEnd();

	glEnable(GL_LIGHTING);
}


void RenderArrow(Vector3 const &start, Vector3 const &end, float width, RGBAColour const &_col/* =RGBAColour */)
{
	Camera *cam = g_app->m_camera;
	Vector3 midPoint = (start + end) * 0.5;
	Vector3 midPointToCamera = cam->GetPos() - midPoint;
	float midPointToCameraDist = midPointToCamera.Mag();
	Vector3 invDir = start - end;
	Vector3 sidewaysDir = ((cam->GetPos() - end) ^ invDir).Normalise();
	float arrowLen = invDir.Mag();
	invDir.SetLength(arrowLen * 0.2);

	if (_col.a != 255)
	{
		glEnable(GL_BLEND);
	}

	glLineWidth(width / midPointToCameraDist * arrowLen);
	glColor3ubv(_col.GetData());

	glBegin(GL_LINES);
		glVertex3dv(start.GetDataConst());
		glVertex3dv(end.GetDataConst());

		Vector3 p1 = end + invDir + sidewaysDir * arrowLen * 0.1;
		Vector3 p2 = end + invDir - sidewaysDir * arrowLen * 0.1;
		glVertex3dv(p1.GetDataConst());
		glVertex3dv(end.GetDataConst());
		glVertex3dv(p2.GetDataConst());
		glVertex3dv(end.GetDataConst());
	glEnd();

    glDisable( GL_LINE_SMOOTH );
	glDisable( GL_BLEND );
}

#endif // ifdef DEBUG_RENDER_ENABLED
