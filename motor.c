#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>

#define FORWARD 1       //Direction for Up, Left
#define REVERSE 0       //Direction for Down, Right
#define PAN 0           //Horizontal
#define TILT 1          //Vertical
#define MAX_H 172       //Max steps horizontal
#define MAX_V 40        //Max steps vertical
#define CENTER_H 86     //Center pos horizontal
#define CENTER_V 20     //Center pos vertical
#define POS_LEN 7       //Format: xxx,yyy
#define MAXPATHLEN 200  //Max Len for full path

//Full path of executable
char fullpath[MAXPATHLEN];

//Current pos
int h = 0;
int v = 0;

//Present target pos
int present_h = 0;
int present_v = 0;

//Store current pos to file
void store_pos(int h, int v) {
    FILE *fp;
    char filename[MAXPATHLEN];

    //construct full pathname
    sprintf(filename, "%s/pos.txt", fullpath);
    fp = fopen(filename, "w");
    fprintf(fp, "%d,%d", h, v);
    fclose(fp);
}

//Store pos to present
void store_present(int present, int h, int v) {
    FILE *fp;
    char filename[MAXPATHLEN];

    //construct full pathname
    sprintf(filename, "%s/present%d.txt", fullpath, present);
    fp = fopen(filename, "w");
    fprintf(fp, "%d,%d", h, v);
    fclose(fp);
}

//Load Present from file
void load_present(int present) {
    FILE *fp;
    char str[POS_LEN]; //Format: xxx,yyy

    //if present no out of bounds, return current pos == do nothing
    if (present < 0 || present > 8) {
        present_h = h;
        present_v = v;
        return;
    }

    //construct full pathname
    char filename[MAXPATHLEN];
    sprintf(filename, "%s/present%d.txt", fullpath, present);
    fp = fopen(filename, "r");
    if (fp == NULL){
        return;
    }
    fgets(str, POS_LEN, fp);
    fclose(fp);

    char* rest = str;
    //split params for h and v
    present_h = atoi(strtok_r(rest, ",", &rest));
    present_v = atoi(strtok_r(rest, ",", &rest));
}

// Move motor and update pos
void motor_move(int motor, int direction, int steps) {
    motor_init();
    switch (motor) {
        case PAN:
            motor_h_dir_set(direction);
            motor_h_dist_set(steps);
            motor_h_move();

            if (direction == FORWARD) {
                h = h + steps;
            } else if (direction == REVERSE) {
                h = h - steps;
            }
        break;
        case TILT:
            motor_v_dir_set(direction);
            motor_v_dist_set(steps);
            motor_v_move();

            if (direction == FORWARD) {
                v = v + steps;
            } else if (direction == REVERSE) {
                v = v - steps;
            }
        break;
        default:
            //nothing to do
        break;
    }
    motor_exit();
}

//Move motor left and obey limits
void motor_left(int steps) {
    if (h + steps > MAX_H) {
        steps = MAX_H-h;
    }
    motor_move(PAN, FORWARD, steps);
    store_pos(h, v);
}

//Move motor right and obey limits
void motor_right(int steps) {
    if (h-steps < 0) {
        steps = h;
    }
    motor_move(PAN, REVERSE, steps);
    store_pos(h, v);
}

//Move motor up and obey limits
void motor_up(int steps) {
    if (v + steps > MAX_V) {
        steps = MAX_V-v;
    }
    motor_move(TILT, FORWARD, steps);
    store_pos(h, v);
}

//Move motor down and obey limits
void motor_down(int steps) {
    if (v-steps < 0) {
        steps = v;
    }
    motor_move(TILT, REVERSE, steps);
    store_pos(h, v);
}

//Move motor to specified pos and obey limits
void motor_goto(int present_h, int present_v) {
    //Calculate req. steps from current pos in horizontal axis and move accordingly
    if (present_h > h) {
        motor_left(present_h-h);
    } else if ( present_h < h) {
        motor_right(h-present_h);
    }

    //Calculate req. steps from current pos in vertical axis and move accordingly
    if (present_v > v) {
        motor_up(present_v-v);
    } else if ( present_v < v) {
        motor_down(v-present_v);
    }
}

//Calibrate motor
void motor_calibrate() {
    //Set internal position to MAX without moving to make sure the functions allow a max # of steps
    h = MAX_H;
    v = MAX_V;

    //calibrate horizontal axis first, right is 0. Move to center afterwards
    motor_right(MAX_H);
    h = 0;
    motor_left(CENTER_H);

    //calibrate vertical axis, down is 0. Move to center afterwards
    motor_down(MAX_V);
    v = 0;
    motor_up(CENTER_V);
}

//Load current pos from file
void load_pos() {
    FILE *fp;
    char str[POS_LEN];

    //construct full pathname
    char filename[MAXPATHLEN];
    sprintf(filename, "%s/pos.txt", fullpath);
    fp = fopen(filename, "r");
    if (fp == NULL){
        printf("No position found, calibrating...");
        motor_calibrate();
        load_pos();
        return;
    }
    fgets(str, POS_LEN, fp);
    fclose(fp);

    char* rest = str;
    ///split params for h and v
    h = atoi(strtok_r(rest, ",", &rest));
    v = atoi(strtok_r(rest, ",", &rest));
}

//Run me!
int main(int argc, char **argv) {
    char command[10];
    int steps_present = 0;

    //Min Args is 2 for calibrate
    if (argc >= 2 ) {
        //Parse command
        strcpy(command, argv[1]);

        //if command != calibrate, we need at least 3 args. Print help otherwise
        if (strcmp(command,"calibrate") != 0) {
            if (argc < 3) {
                char filename[10];
                strcpy(filename, argv[0]);
                printf("Usage: \n%s <calibrate> | <left|right|up|down> <steps> | <store|goto> <present[1-8]>\n", filename);
                exit(1);
            } else {
                //Parse stepcount/present
                steps_present = atoi(argv[2]);
            }
        }
    } else {
        char filename[10];
        strcpy(filename, argv[0]);
        printf("Usage: \n%s <calibrate> | <left|right|up|down> <steps> | <store|goto> <present[1-8]>\n", filename);
        exit(1);
    }

    //get path of executable in order to keep the pos files within the same dir
    int length;
    char *p;
    length = readlink("/proc/self/exe", fullpath, sizeof(fullpath));

    //Catch some errors
    if (length < 0) {
        perror("resolving symlink /proc/self/exe.");
        exit(1);
    }
    if (length >= sizeof(fullpath)) {
        fprintf(stderr, "Path too long.\n");
        exit(1);
    }

    //remove executable name from path
    if((p = strrchr(fullpath, '/')))
        *(p+1) = '\0';
    printf("Full path is: %s\n", fullpath);

    //Actions
    if (strcmp(command,"calibrate") == 0) {
        motor_calibrate();
    } else {
        //Load current pos from file or calibrate
        load_pos();

        if (strcmp(command,"left") == 0) {
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
}

