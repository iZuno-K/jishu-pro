all: niconico_server niconico_client clientTest serverTest

#OPENCV_CFLAGS = `pkg-config opencv-2.3.1 --cflags`
#OPENCV_LIBS   = `pkg-config opencv-2.3.1 --libs`
OPENCV_CFLAGS = `pkg-config opencv --cflags`
OPENCV_LIBS   = `pkg-config opencv --libs`
GLUT_CFLAGS   = `pkg-config gl --cflags` `pkg-config glu --cflags`
GLUT_LIBS     = `pkg-config gl --libs` `pkg-config glu --libs` -lglut

niconico_server: niconico_server.c
	gcc -pthread -o $@ $< ${OPENCV_CFLAGS} ${OPENCV_LIBS}
miconServer: miconServer.c
	gcc -pthread -o $@ $< ${OPENCV_CFLAGS} ${OPENCV_LIBS}
miconClient: miconClient.c
	gcc -pthread -o $@ $< ${OPENCV_CFLAGS} ${OPENCV_LIBS}
cvtest: cvtest.cpp
	g++ -o $@ $< ${OPENCV_CFLAGS} ${OPENCV_LIBS}
niconico_client: niconico_client.c
	gcc -pthread -o $@ $< ${OPENCV_CFLAGS} ${OPENCV_LIBS} ${GLUT_CFLAGS} ${GLUT_LIBS}


serverTest: serverTest.c
	g++ -pthread -o $@ $< ${OPENCV_CFLAGS} ${OPENCV_LIBS}
clientTest: clientTest.c
	gcc -pthread -o $@ $< ${OPENCV_CFLAGS} ${OPENCV_LIBS} ${GLUT_CFLAGS} ${GLUT_LIBS}
