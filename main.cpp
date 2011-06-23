#include <QtCore/QCoreApplication>
#include <QFile>
#include <QDebug>

#include "dump.h"

typedef struct {
	QString		inputName ;
	QByteArray	inputData ;
	QString		outputName ;
} Work ;
Work work ;

void print_usage()
{
	printf("usage : swfdump -i:input_file") ;
}

bool check_opt( int argc, char *argv[] )
{
	for ( int i = 1 ; i < argc ; i ++ ) {
		if ( argv[i][0] != '-' ) { continue ; }
		switch ( argv[i][1] ) {
		case 'i':
		case 'I':
			{
				char *p = &argv[i][3] ;
				work.inputName = p ;
			}
			break ;
		case 'o':
		case 'O':
			{
				char *p = &argv[i][3] ;
				work.outputName = p ;
			}
			break ;
		}
	}
	if ( work.inputName.isEmpty() ) {
		printf("input file not found") ;
		return false ;
	}
	return true ;
}

bool convert( QByteArray &ret )
{
	char *data = work.inputData.data() ;
	if ( !data ) {
		printf("data not found\n") ;
		return false ;
	}
	if ( data[0] != 'C'
	  || data[1] != 'W'
	  || data[2] != 'S' ) {
		return false ;
	}
	int fileSize = *((unsigned int *)(&data[4])) ;
	int compSize = work.inputData.length() - 8 ;
	compSize = (compSize&0xff)<<24 | (compSize&0xff00)<<8 | (compSize&0xff0000)>>8 | (compSize&0xff000000)>>24 ;
	QByteArray comp ;
	comp.append((char *)&compSize, 4) ;
	comp += work.inputData.right(work.inputData.length()-8) ;
	QByteArray uncomp = qUncompress(comp) ;

	ret[0] = 'F' ;
	ret[1] = 'W' ;
	ret[2] = 'S' ;
	ret[3] = data[3] ;
	ret.append((char *)&fileSize, 4) ;
	ret += uncomp ;

	return true ;
}

bool read( void )
{
	QFile file(work.inputName) ;
	if ( !file.open(QFile::ReadOnly) ) {
		printf("file open failed : %s\n", work.inputName.toStdString().c_str()) ;
		return false ;
	}
	work.inputData = file.readAll() ;
	file.close() ;
	return true ;
}

int main(int argc, char *argv[])
{
	if ( !check_opt(argc, argv) ) {
		print_usage() ;
		return 1 ;
	}
	if ( !read() ) {
		printf("read failed\n") ;
		return 1 ;
	}
	QByteArray uncomp ;
	if ( convert(uncomp) ) {
		work.inputData = uncomp ;
	}
	QString ret = dump(work.inputData) ;
	qDebug() << ret ;
	if ( work.outputName.length() ) {
		QFile file(work.outputName) ;
		if ( file.open(QFile::WriteOnly) ) {
			file.write(ret.toStdString().c_str()) ;
			file.close();
		}
	}

	return 0 ;
}
