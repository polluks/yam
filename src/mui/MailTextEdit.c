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

 Superclass:  MUIC_TextEditor
 Description: Our own inherented version of TextEditor.mcc

***************************************************************************/

#include "MailTextEdit_cl.h"

#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"

#include "MUIObjects.h"
#include "Requesters.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *slider;
  LONG    colorMap[16];

  struct MUI_EventHandlerNode ehnode;

  BOOL eventHandlerAdded;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = (Object *)DoSuperMethodA(cl, obj, msg)))
  {
    GETDATA;
    struct TagItem *tags = inittags(msg), *tag;

    // set the colormap
    set(obj, MUIA_TextEditor_ColorMap, data->colorMap);

    while((tag = NextTagItem((APTR)&tags)))
    {
      switch(tag->ti_Tag)
      {
        // we also catch foreign attributes
        case MUIA_TextEditor_Slider: data->slider = (Object *)tag->ti_Data; break;
      }
    }
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_DragQuery)
OVERLOAD(MUIM_DragQuery)
{
  struct MUIP_DragDrop *drop_msg = (struct MUIP_DragDrop *)msg;

  return (ULONG)(drop_msg->obj == G->AB->GUI.LV_ADDRESSES);
}

///
/// OVERLOAD(MUIM_DragDrop)
OVERLOAD(MUIM_DragDrop)
{
  struct MUIP_DragDrop *drop_msg = (struct MUIP_DragDrop *)msg;

  if(drop_msg->obj == G->AB->GUI.LV_ADDRESSES)
  {
    struct MUI_NListtree_TreeNode *tn;

    if((tn = (struct MUI_NListtree_TreeNode *)xget(drop_msg->obj, MUIA_NListtree_Active)))
    {
      struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);

      if(ab->Type != AET_GROUP)
      {
        char address[SIZE_LARGE];

        BuildAddress(address, sizeof(address), ab->Address, ab->RealName);
        DoMethod(obj, MUIM_TextEditor_InsertText, address, MUIV_TextEditor_InsertText_Cursor);
      }
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_Show)
OVERLOAD(MUIM_Show)
{
  GETDATA;
  IPTR result;

  ENTER();

  data->colorMap[6] = MUI_ObtainPen(muiRenderInfo(obj), &C->ColoredText, 0);
  data->colorMap[7] = MUI_ObtainPen(muiRenderInfo(obj), &C->Color1stLevel, 0);
  data->colorMap[8] = MUI_ObtainPen(muiRenderInfo(obj), &C->Color2ndLevel, 0);
  data->colorMap[9] = MUI_ObtainPen(muiRenderInfo(obj), &C->Color3rdLevel, 0);
  data->colorMap[10] = MUI_ObtainPen(muiRenderInfo(obj), &C->Color4thLevel, 0);
  data->colorMap[11] = MUI_ObtainPen(muiRenderInfo(obj), &C->ColorURL, 0);
  data->colorMap[12] = MUI_ObtainPen(muiRenderInfo(obj), &C->ColorSignature, 0);

  // call the supermethod
  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_Hide)
OVERLOAD(MUIM_Hide)
{
  GETDATA;
  ULONG result;

  ENTER();

  // call the super method first
  if((result = DoSuperMethodA(cl, obj, msg)) == 0)
  {
    int i;

    // release all pens of our own colorMap
    for(i=6; i <= 12; i++)
    {
      if(data->colorMap[i] >= 0)
      {
        MUI_ReleasePen(muiRenderInfo(obj), data->colorMap[i]);
        data->colorMap[i] = -1;
      }
    }
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_Setup)
// On the Setup of the TextEditor gadget we prepare the evenhandlernode
// for adding it later on a GoActive Method call.
OVERLOAD(MUIM_Setup)
{
  GETDATA;
  IPTR result;

  ENTER();

  if((result = DoSuperMethodA(cl, obj, msg)))
  {
    data->ehnode.ehn_Priority = 1;
    data->ehnode.ehn_Flags    = 0;
    data->ehnode.ehn_Object   = obj;
    data->ehnode.ehn_Class    = cl;
    data->ehnode.ehn_Events   = IDCMP_RAWKEY;

    DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);
    data->eventHandlerAdded = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_Cleanup)
// On a Cleanup we have to remove the EventHandler
OVERLOAD(MUIM_Cleanup)
{
  GETDATA;
  IPTR result;

  ENTER();

  if(data->eventHandlerAdded == TRUE)
  {
    DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);
    data->eventHandlerAdded = FALSE;
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_HandleEvent)
// We use HandleEvent to implement our neat tiny feature that on a press of
// RAMIGA+DEL while a multiline text is marked YAM deletes the text and
// inserts the famous [...] substitution.
OVERLOAD(MUIM_HandleEvent)
{
  struct IntuiMessage *imsg;

  if((imsg = ((struct MUIP_HandleEvent *)msg)->imsg))
  {
    if(imsg->Class == IDCMP_RAWKEY)
    {
      if(imsg->Code == IECODE_DEL)
      {
        if(isFlagSet(imsg->Qualifier, IEQUALIFIER_RCOMMAND) &&
           !xget(obj, MUIA_TextEditor_ReadOnly))
        {
          ULONG ret;
          ULONG x1, y1, x2, y2;

          // let`s check first if a multiline block is marked or not
          if(DoMethod(obj, MUIM_TextEditor_BlockInfo, &x1, &y1, &x2, &y2) && y2-y1 >= 1)
          {
            // then we first clear the qualifier so that the real
            // TextEditor HandleEvent method treats this imsg as a normal DEL pressed imsg
            CLEAR_FLAG(imsg->Qualifier, IEQUALIFIER_RCOMMAND);
            ret = DoSuperMethodA(cl, obj, msg);

            // Now that the marked text is cleared we can insert our great [...]
            // snip text ;)
            DoMethod(obj, MUIM_TextEditor_InsertText, "[...]\n", MUIV_TextEditor_InsertText_Cursor);

            return ret;
          }
        }
      }
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_TextEditor_HandleError)
OVERLOAD(MUIM_TextEditor_HandleError)
{
  const char *errortxt = NULL;

  ENTER();

  SHOWVALUE(DBF_GUI, ((struct MUIP_TextEditor_HandleError *)msg)->errorcode);

  switch(((struct MUIP_TextEditor_HandleError *)msg)->errorcode)
  {
    case Error_ClipboardIsEmpty:
    case Error_ClipboardIsNotFTXT:
    case Error_NoAreaMarked:
    case Error_NothingToRedo:
    case Error_NothingToUndo:
      // nothing but DisplayBeep()
    break;

    case Error_NotEnoughUndoMem:
      errortxt = tr(MSG_WR_ErrorNotEnoughUndoMem);
    break;
  }

  if(errortxt)
    MUI_Request(_app(obj), _win(obj), 0L, NULL, tr(MSG_OkayReq), errortxt);
  else
    DisplayBeep(NULL);

  LEAVE();
  return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Public Methods */
