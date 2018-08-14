/*
 * NeoPixel.hpp
 *
 *  Created on: May 21, 2018
 *      Author: aksha
 */

#ifndef NEOPIXEL_HPP_
#define NEOPIXEL_HPP_

enum color{
  RED,
  GREEN,
  BLUE,
  WHITE,
  OFF,
  CUSTOM
};

void bit(int bit_rx){
    if(bit_rx == 1){
        neoRing.setHigh();neoRing.setHigh();neoRing.setLow();delay_us(1);
    }
    else if(bit_rx == 0){
        neoRing.setHigh();neoRing.setLow();delay_us(1);
    }
}

void reset(){
    neoRing.setLow();
    delay_us(60);
}

void off(){
    for (int i = 0; i < 32; i++) {
        bit(0);
    }
}

void red(){
    for (int i = 0; i < 32; i++) {
        if (i < 8 || i > 15) {
            bit(0);
        }
        else {
            bit(1);
        }
    }
}

void green(){
    for (int i = 0; i < 32; i++) {
        if (i < 8) {
            bit(1);
        }
        else {
            bit(0);
        }
    }
}

void blue(){
    for (int i = 0; i < 32; i++) {
        if (i < 16 || i > 23) {
            bit(0);
        }
        else {
            bit(1);
        }
    }
}

void white(){
    for (int i = 0; i < 32; i++) {
            if (i < 24) {
                bit(1);
            }
            else {
                bit(0);
            }
        }
}

void refresh(){
    reset();
    for (int i = 0; i < 24; i++) {
                off();
            }
    reset();
}

void setColor(color C){
    switch(C){
        case 0: red(); break;
        case 1: green(); break;
        case 2: blue(); break;
        case 3: white(); break;
        case 4: off(); break;
        //case '5': custom(); break;
        default: off(); break;
    }
}

void selectLed(uint8_t n, color C){
    if(! (n <= 0 || n > 25)){
    refresh();
    for (int i = 1; i < 25; i++) {
        if(i == n){
            setColor(C);
        }
        else{
            off();
        }
    }
    reset();
    }
    else{
        printf(" Invalid neoPixel Number \n");
    }
}

void cascadeLed(uint8_t start, uint8_t end, color C){ // color array
    refresh();// ~161 us
    bool flag_exception = false;
    if(start > end){
        flag_exception = true;
    }
    for (int i = 1; i < 25; i++) {
        if((i >= start && i <= end) && !flag_exception){
            setColor(C);    // ~44us
        }
        else if((i >= start || i <= end ) && flag_exception){
            setColor(C);    // ~44us
        }
        else{
            off();          // ~41us
        }
    }
    reset();
    //delay_ms(100); // to be removed

}

void neo(uint8_t start, uint8_t end, color C){
        cascadeLed(start, end, C);
}

void begin(){
    printf("inside begin \n");
    for(int i = 1; i < 25; i++) {
        selectLed(i, WHITE);
        delay_ms(100);
    }
    cascadeLed(1, 24, WHITE);
    //delay_ms(1000);
}

void right(){
    neo(1, 12, GREEN);
}

void left(){
    neo(13, 24, RED);
}

void forward(){
    neo(19, 6, WHITE);
}

void backward(){
    neo(7, 18, BLUE);
}



#endif /* NEOPIXEL_HPP_ */
