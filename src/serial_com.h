#ifndef DATAVIEWER_SERIAL_COM_H
#define DATAVIEWER_SERIAL_COM_H
// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <glad.h>
#include <iostream>

using namespace std;

//const int ROLLING_BUFFER_LEN = 1024 * 1024;
const int ROLLING_BUFFER_LEN = 2048;
const float FACTOR = 512. * 512. * 5;
//const float FACTOR = 200000.;

float *rolling_samples;
int rolling_index = 0;
int rolling_serial_port;
uint rollingVAO, rollingBAO;

void *fill_buffer(void *port) {
    while (true) {
        int x = 0;
        uint32_t n = read(rolling_serial_port, &x, sizeof x);
        if (n < 0) {
            cout << "Error reading: " << strerror(errno) << endl;
            break;
        }
        cout << "Read: " << x << endl;
        rolling_samples[rolling_index++] = x / FACTOR;
        rolling_index %= ROLLING_BUFFER_LEN;
    }
    close(rolling_serial_port);
}

void rolling_signal_destroy() {
    close(rolling_serial_port);
}

void serial_com_init(string device) {
    rolling_serial_port = open(("/dev/" + device).c_str(), O_RDWR);

    if (rolling_serial_port < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
    }
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(rolling_serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    }
    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);
    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
                     ICRNL); // Disable any special handling of received bytes
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    tcflush(rolling_serial_port, TCIFLUSH);
    if (tcsetattr(rolling_serial_port, TCSANOW, &tty) != 0) {
        cout << "Error " << errno << " from tcsetattr" << endl;
    }

    rolling_samples = (float *) calloc(ROLLING_BUFFER_LEN, sizeof(float));
    glGenVertexArrays(1, &rollingVAO);
    glGenBuffers(1, &rollingBAO);
    glBindVertexArray(rollingVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rollingBAO);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 4, 0);
    glEnableVertexAttribArray(0);

    pthread_t pthread;
    int ret_code = pthread_create(&pthread, NULL, &fill_buffer, NULL);
}

#endif //DATAVIEWER_SERIAL_COM_H
