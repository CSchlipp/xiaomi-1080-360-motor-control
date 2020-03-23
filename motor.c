  #include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>


#define FORWARD 1 //Up, Left
#define REVERSE 0 //Down, Right
#define PAN 0 //H
#define TILT 1 //V
#define MAX_H 172
#define MAX_V 40
#define CENTER_H 86
#define CENTER_V 20

#define MAXPATHLEN 200

char fullpath[MAXPATHLEN];

int h = 0;
int v = 0;

int present_h = 0;
int present_v = 0;

void store_pos(int h, int v) {
    FILE *fp;
    char filename[MAXPATHLEN];
    sprintf(filename, "%s/pos.txt", fullpath);
    fp = fopen(filename, "w");
    fprintf(fp, "%d,%d", h, v);
    fclose(fp);
}

void load_pos() {
    FILE *fp;
    char str[7];
    char filename[MAXPATHLEN];
    sprintf(filename, "%s/pos.txt", fullpath);

    fp = fopen(filename, "r");
    if (fp == NULL){
        return;
    }
    fgets(str, 7, fp);
    fclose(fp);

    char* rest = str;
    //split params
    h = atoi(strtok_r(rest, ",", &rest));
    v = atoi(strtok_r(rest, ",", &rest));
    printf("h %d, v %d", h, v);
}

void store_present(int present, int h, int v) {
    FILE *fp;
    char filename[MAXPATHLEN];
    sprintf(filename, "%s/present%d.txt", fullpath, present);
    fp = fopen(filename, "w");
    fprintf(fp, "%d,%d", h, v);
    fclose(fp);
}

void load_present(int present) {
    FILE *fp;
    char str[7];
    if (present < 0 || present > 8) {
        present_h = 0;
        present_v = 0;
        return;
    }
    char filename[MAXPATHLEN];
    sprintf(filename, "%s/present%d.txt", fullpath, present);


    fp = fopen(filename, "r");
    if (fp == NULL){
        return;
    }
    fgets(str, 7, fp);
    fclose(fp);
    char* rest = str;
    //split params
    present_h = atoi(strtok_r(rest, ",", &rest));
    present_v = atoi(strtok_r(rest, ",", &rest));
    printf("h %d, v %d", present_h, present_v);
}

void motor_move(int motor, int direction, int steps) {
    motor_init();
    switch (motor) {
        case 0:
            motor_h_dir_set(direction);
            motor_h_dist_set(steps);
            motor_h_move();

            if (direction == FORWARD) {
                h = h + steps;
            } else if (direction == REVERSE) {
                h = h - steps;
            }
            break;
        case 1:
            motor_v_dir_set(direction);
            motor_v_dist_set(steps);
            motor_v_move();

            if (direction == FORWARD) {
                v = v + steps;
            } else if (direction == REVERSE) {
                v = v - steps;
            }
            break;
    }
    motor_exit();
}

void motor_left(int steps) {
    if (h + steps > MAX_H) {
        steps = MAX_H-h;
    }
    motor_move(PAN, FORWARD, steps);
    store_pos(h, v);
}

void motor_right(int steps) {
    if (h-steps < 0) {
        steps = h;
    }
    motor_move(PAN, REVERSE, steps);
    store_pos(h, v);
}

void motor_up(int steps) {
    if (v + steps > MAX_V) {
        steps = MAX_V-v;
    }
    motor_move(TILT, FORWARD, steps);
    store_pos(h, v);
}

void motor_down(int steps) {
    if (v-steps < 0) {
        steps = v;
    }
    motor_move(TILT, REVERSE, steps);
    store_pos(h, v);
}

void motor_goto(int present_h, int present_v) {
    if (present_h > h) {
        motor_left(present_h-h);
    } else if ( present_h < h) {
        motor_right(h-present_h);
    }

    if (present_v > v) {
        motor_up(present_v-v);
    } else if ( present_v < v) {
        motor_down(v-present_v);
    }
}

void motor_calibrate() {
    h = MAX_H;
    v = MAX_V;

    motor_right(MAX_H);
    h = 0;
    motor_left(CENTER_H);

    motor_down(MAX_V);
    v = 0;
    motor_up(CENTER_V);
}


int main(int argc, char **argv) {

    if (argc < 3 ) {
        char filename[10];
        strcpy(filename, argv[0]);
        printf("Usage: \n%s <calibrate|left|right|up|down|store|goto> <steps|present[1-8]>\n", filename);
        exit(1);
    }

    char command[10];
    strcpy(command, argv[1]);
    int steps_present = atoi(argv[2]);

    int length;
    char *p;
    length = readlink("/proc/self/exe", fullpath, sizeof(fullpath));

    /*
     * Catch some errors:
     */
    if (length < 0) {
        perror("resolving symlink /proc/self/exe.");
        exit(1);
    }
    if (length >= sizeof(fullpath)) {
        fprintf(stderr, "Path too long.\n");
        exit(1);
    }
    if((p = strrchr(fullpath, '/')))
        *(p+1) = '\0';
    printf("Full path is: %s\n", fullpath);

    load_pos();

    if (strcmp(command,"calibrate") == 0) {
        motor_calibrate();
    }
    else if (strcmp(command,"left") == 0) {
        motor_left(steps_present);
    }
    else if (strcmp(command,"right") == 0) {
        motor_right(steps_present);
    }
    else if (strcmp(command,"up") == 0) {
        motor_up(steps_present);
    }
    else if (strcmp(command,"down") == 0) {
        motor_down(steps_present);
    }
    else if (strcmp(command,"store") == 0 && steps_present > 0 && steps_present <=8) {
        store_present(steps_present, h, v);
    }
    else if (strcmp(command,"goto") == 0 && steps_present > 0 && steps_present <=8) {
        load_present(steps_present);
        motor_goto(present_h, present_v);
    }
}

