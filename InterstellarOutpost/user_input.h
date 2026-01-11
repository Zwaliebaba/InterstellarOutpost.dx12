#ifndef USER_INPUT_H
#define USER_INPUT_H

#include "llist.h"
#include "vector3.h"

#include "worldobject.h"

class StretchyIcons;
class Building;
class Engineer;


class UserInput
{
public:
	bool		m_removeTopLevelMenu;

private:
	Vector3		m_mousePos3d;

  void	AdvanceMenus();

    LList   <Vector3 *> m_mousePosHistory;
	
public:
    UserInput();
    void	Advance();
    void	Render();

	void	RecalcMousePos3d();			// Updates the cached value of m_mousePos3d by doing a ray cast against landscape
    Vector3 GetMousePos3d();            // Returns the cached value "m_mousePos3d"
};


#endif
