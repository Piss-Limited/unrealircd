/*
 *   IRC - Internet Relay Chat, src/modules/m_setident.c
 *   (C) 1999-2001 Carsten Munk (Techie/Stskeeps) <stskeeps@tspre.org>
 *
 *   See file AUTHORS in IRC package for additional names of
 *   the programmers. 
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "unrealircd.h"

#define MSG_SETIDENT 	"SETIDENT"	/* set ident */

CMD_FUNC(m_setident);

ModuleHeader MOD_HEADER(m_setident)
  = {
	"setident",	/* Name of module */
	"4.2", /* Version */
	"/setident", /* Short description of module */
	"3.2-b8-1",
	NULL 
    };

MOD_INIT(m_setident)
{
	CommandAdd(modinfo->handle, MSG_SETIDENT, m_setident, MAXPARA, M_USER);
	MARK_AS_OFFICIAL_MODULE(modinfo);
	return MOD_SUCCESS;
}

MOD_LOAD(m_setident)
{
	return MOD_SUCCESS;
}

MOD_UNLOAD(m_setident)
{
	return MOD_SUCCESS;
}

/* m_setident - 12/05/1999 - Stskeeps
 *  :prefix SETIDENT newident
 *  parv[1] - newident
 *  D: This will set your username to be <x> (like (/setident Root))
 *     (if you are IRCop) **efg*
 *     Very experimental currently
 * 	   Cloning of m_sethost at some points - so same authors ;P
*/
CMD_FUNC(m_setident)
{

	char *vident, *s;
#ifndef DISABLE_USERMOD
	int  permit = 0;	/* 0 = opers(glob/locop) 1 = global oper */
#else
	int  permit = 2;
#endif
	int  legalident = 1;	/* is legal characters? */
	if (!MyConnect(sptr))
		goto permit_2;
	switch (permit)
	{
	  case 0:
	  case 1:
		  if (!ValidatePermissionsForPath("self:set:ident",sptr,NULL,NULL,NULL))
		  {
			  sendnumeric(sptr, ERR_NOPRIVILEGES, me.name,
			      sptr->name);
			  return 0;
		  }
		  break;
	  case 2:
		  if (MyConnect(sptr))
		  {
			  sendnumeric(sptr, ERR_NOPRIVILEGES, me.name,
			      sptr->name);
			  return 0;
		  }
		  break;
	  default:
		  sendto_ops_butone(IsServer(cptr) ? cptr : NULL, sptr,
		      ":%s WALLOPS :[SETIDENT] Somebody fixing this corrupted server? !(0|1) !!!",
		      me.name);
		  break;
	}
      permit_2:
	if (parc < 2)
		vident = NULL;
	else
		vident = parv[1];

	if (BadPtr(vident))
	{
		if (MyConnect(sptr))
			sendnotice(sptr, "*** Syntax: /SetIdent <new ident>");
		return 1;
	}

	if (strlen(vident) > (USERLEN))
	{
		if (MyConnect(sptr))
			sendnotice(sptr, "*** /SetIdent Error: Usernames are limited to %i characters.", USERLEN);
		return 0;
	}

	/* illegal?! */
	for (s = vident; *s; s++)
	{
		if ((*s == '~') && (s == vident))
			continue;
		if (!isallowed(*s))
		{
			legalident = 0;
			break;
		}
	}

	if (legalident == 0)
	{
		sendnotice(sptr, "*** /SetIdent Error: A username may contain a-z, A-Z, 0-9, '-', '~' & '.'.");
		return 0;
	}

	userhost_save_current(sptr);

	switch (UHOST_ALLOWED)
	{
		case UHALLOW_ALWAYS:
			break;
		case UHALLOW_NEVER:
			if (MyClient(sptr))
			{
				sendnotice(sptr, "*** /SetIdent is disabled");
				return 0;
			}
			break;
		case UHALLOW_NOCHANS:
			if (MyClient(sptr) && sptr->user->joined)
			{
				sendnotice(sptr, "*** /SetIdent can not be used while you are on a channel");
				return 0;
			}
			break;
		case UHALLOW_REJOIN:
			/* dealt with later */
			break;
	}

	/* get it in */
	ircsnprintf(sptr->user->username, sizeof(sptr->user->username), "%s", vident);
	/* spread it out */
	sendto_server(cptr, 0, 0, NULL, ":%s SETIDENT %s", sptr->name, parv[1]);

	userhost_changed(sptr);

	if (MyConnect(sptr))
	{
		sendnotice(sptr,
		    "Your nick!user@host-mask is now (%s!%s@%s) - To disable ident set change it manually by /setident'ing again",
		    sptr->name, sptr->user->username, GetHost(sptr));
	}
	return 0;
}
