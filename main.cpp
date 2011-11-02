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
