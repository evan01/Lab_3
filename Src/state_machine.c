

typedef enum {
	INPUT_MODE,
	OPERATION_MODE,

}general_state_e;

typedef enum {
	SLEEP_STATE,
	START_STATE,
	ENTER_ROLL_STATE,
	ENTER_PITCH_STATE,
	PITCH_MONITOR_STATE,
	ROLL_MONITOR_STATE
}state_e;

typedef enum {
	HASHTAG,
	STAR,
	NUMBER_1,
	NUMBER_2,
	NUMBER_3,
	NUMBER_4,
	NUMBER_5,
	NUMBER_6,
	NUMBER_7,
	NUMBER_8,
	NUMBER_9,
	NUMBER_0
}event_e;

typedef enum {
	REGULAR,
	MID_PRESS,
	LONG_PRESS
} press_type_e;

state_e state = START_STATE;
state_e next_state;
press_type_e press_type;
event_e event;

int setPressType(int duration){
	switch(duration){
		case 0: 
			press_type = REGULAR;  
			break;
		case 1:
			press_type = MID_PRESS;
			break;
		case 2:
			press_type = MID_PRESS;
			break;
		case 3:
			press_type = LONG_PRESS;
			break;
		default:
			press_type = REGULAR;
			break;
	}
	return 0;
}

int setEvent(int digit){
	switch(digit){
		case 0:
			event = NUMBER_0;
			break;
		case 1:
			event = NUMBER_1;
			break;
		case 2:
			event = NUMBER_2;
			break;
		case 3:
			event = NUMBER_3;
			break;
		case 4:
			event = NUMBER_4;
			break;
		case 5:
			event = NUMBER_5;
			break;
		case 6:
			event = NUMBER_6;
			break;
		case 7:
			event = NUMBER_7;
			break;
		case 8:
			event = NUMBER_8;
			break;
		case 9:
			event = NUMBER_9;
			break;
		case 10:
			event = STAR;
			break;
		case 11:
			event = HASHTAG;
			break;
	}
	return 0;
}

int getKeypadInput(int digit, int duration){
	setPressType(duration);
	setEvent(digit);
}

int set_state(){
	if(state == START_STATE){
		state = ENTER_ROLL_STATE;
	}
	if(press_type == LONG_PRESS){
		if(event == STAR){
			next_state = SLEEP_STATE;
		}else if(event == HASHTAG && state == SLEEP_STATE){
			next_state = START_STATE;
		}else{
			next_state = state;
		}
	}else if(press_type == MID_PRESS){
		if(event == STAR && state != SLEEP_STATE){
			next_state = START_STATE;
		}else{
			next_state = state;
		}
	}else{
		switch(state){
			case START_STATE:
				next_state = ENTER_ROLL_STATE;
			case SLEEP_STATE:
				next_state = SLEEP_STATE;
				break;
			case ENTER_ROLL_STATE:
				if(event == HASHTAG){
					next_state = ENTER_PITCH_STATE;
				}else if(event == STAR){
					//clear last digit
				}else{
					//add number to roll variable
				}
				break;
			case ENTER_PITCH_STATE:
				if(event == HASHTAG){
					next_state = PITCH_MONITOR_STATE;
				}else if(event == STAR){
					//clear last digit
				}else{
					//add number to pitch variable
				}
				break;
			case PITCH_MONITOR_STATE:
				if(event == NUMBER_1){
					next_state = PITCH_MONITOR_STATE;
				}else if(event == NUMBER_2){
					next_state = ROLL_MONITOR_STATE;
				}else{
					next_state = PITCH_MONITOR_STATE;
				}
				break;
			case ROLL_MONITOR_STATE:
				if(event == NUMBER_1){
					next_state = PITCH_MONITOR_STATE;
				}else if(event == NUMBER_2){
					next_state = ROLL_MONITOR_STATE;
				}else{
					next_state = PITCH_MONITOR_STATE;
				}
				break;
		}
		state = next_state;
	}
}