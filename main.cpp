//
// This file is part of Piqs.
// 
// Piqs is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Piqs is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Piqs.  If not, see <http://www.gnu.org/licenses/>.
//
#include <QtGui/QApplication>
#include "piqs.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	{
		QStringList args = a.arguments();
		QString root;
		if(args.length()>1) {
			root = args.at(1);
		}
		Piqs *w = new Piqs(root);
		w->show();
	}

    return a.exec();
}
