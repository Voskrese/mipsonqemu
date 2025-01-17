/*
Launchy: Application Launcher
Copyright (C) 2007  Josh Karlin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef runner_H
#define runner_H
#include "gui.h"
#include "globals.h"

#include "plugin_interface.h"
#include <boost/shared_ptr.hpp>

using namespace boost;

class runnerPlugin : public QObject, public PluginInterface
{
	Q_OBJECT
	Q_INTERFACES(PluginInterface)


public:
	uint HASH_WEBSITE;
	uint HASH_runner;
	QList<runnerCmd> cmds;

private:
	shared_ptr<Gui> gui;
public:
	QString libPath;
	runnerPlugin() {
		gui.reset();
//		gui = NULL;
		HASH_runner = qHash(QString("runner"));
	}
	~runnerPlugin() {}
	int msg(int msgId, void* wParam = NULL, void* lParam = NULL); 

	void setPath(QString * path);
	void getID(uint*);
	void getName(QString*);
	void getCatalog(QList<CatItem>* items);
	void getResults(QList<InputData>* id, QList<CatItem>* results);
	void launchItem(QList<InputData>*, CatItem*);
	void doDialog(QWidget* parent, QWidget**);
	void endDialog(bool accept);
	void init();
	QString getIcon();

};

extern runnerPlugin* grunnerInstance;



#endif
