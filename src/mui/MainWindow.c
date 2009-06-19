/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2009 by YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Window
 Description: Mainwindow class carrying all main GUI elements of YAM

***************************************************************************/

#include "MainWindow_cl.h"

#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  short dummy;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  IPTR result = 0;
  struct TagItem *tags = inittags(msg), *tag;

  ENTER();

  while((tag = NextTagItem((APTR)&tags)))
  {
    switch(tag->ti_Tag)
    {
      case MUIA_Window_DefaultObject:
      {
        // if the user clicks somewhere where the default
        // object would be set to NULL we make sure we set
        // it back to the default object of the readmail group
        if((Object *)tag->ti_Data == NULL)
          tag->ti_Data = xget(G->MA->GUI.PG_MAILLIST, MUIA_MainMailListGroup_ActiveListObject);
      }
      break;
    }
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}
///
/// OVERLOAD(MUIM_Window_Snapshot)
OVERLOAD(MUIM_Window_Snapshot)
{
  IPTR result;

  ENTER();

  // get the weights according to their GUI elements
  G->Weights[0] = xget(G->MA->GUI.LV_FOLDERS,  MUIA_HorizWeight);
  G->Weights[1] = xget(G->MA->GUI.GR_MAILVIEW, MUIA_HorizWeight);
  G->Weights[6] = xget(G->MA->GUI.PG_MAILLIST, MUIA_VertWeight);

  // if the embedded read pane objects are currently active we save their weight values
  if(C->EmbeddedReadPane)
  {
    G->Weights[7] = xget(G->MA->GUI.MN_EMBEDDEDREADPANE, MUIA_VertWeight);
    G->Weights[8] = xget(G->MA->GUI.MN_EMBEDDEDREADPANE, MUIA_ReadMailGroup_HGVertWeight);
    G->Weights[9] = xget(G->MA->GUI.MN_EMBEDDEDREADPANE, MUIA_ReadMailGroup_TGVertWeight);
  }

  // make sure the layout is saved
  SaveLayout(TRUE);

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(DisposeSubWindow)
// method that is used by modeless subwindows to get disposed upon
// their close
DECLARE(DisposeSubWindow) // Object *win
{
  ENTER();

  D(DBF_GUI, "Dispose subwindow: %08lx", msg->win);

  set(msg->win, MUIA_Window_Open, FALSE);
  DoMethod(G->App, OM_REMMEMBER, msg->win);
  MUI_DisposeObject(msg->win);

  RETURN(0);
  return 0;
}

///
