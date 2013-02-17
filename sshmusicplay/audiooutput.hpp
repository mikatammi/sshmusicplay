#ifndef AUDIOOUTPUT_HPP
#define AUDIOOUTPUT_HPP

#include <QObject>

class AudioOutput : public QObject
{
    Q_OBJECT
public:
    explicit AudioOutput(QObject *parent = 0);
    
signals:
    
public slots:
    
};

#endif // AUDIOOUTPUT_HPP
