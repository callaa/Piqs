#ifndef TAGVALIDATOR_H
#define TAGVALIDATOR_H

#include <QValidator>

//! A validator for tag entry boxes
class TagValidator : public QValidator
{
    Q_OBJECT
public:
    explicit TagValidator(QObject *parent = 0);

	QValidator::State validate(QString& input, int &pos) const;
signals:

public slots:

};

#endif // TAGVALIDATOR_H
