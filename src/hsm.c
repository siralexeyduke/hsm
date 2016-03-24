/**
  * @file    hsm.c
  * @author  Alexey Serdyuk
  * @version V0.1.0
  * @date    15-July-2015
  * @brief   Hierarchical state machine implementation.
	*/

#include "hsm.h"

/* Private variables */
const state_t *up_path[16];
const state_t *down_path[16];

/* Private functions declarations */
static eventQueueError_t getEvent( eventQueue_t * eventQueue, event_t * event );
static eventQueueError_t putEvent( eventQueue_t * eventQueue, event_t event );

/**
 * @brief  initEventQueue
 *         Initializes event queue
 * @param  eventQueue: eventQueue_t instance
 * @param  queue: buffer for events
 * @param  length: length of event queue
 * @retval None
 */
void initEventQueue( eventQueue_t * eventQueue, event_t * queue, uint8_t length ){
	eventQueue->queue = queue;
	eventQueue->length = length;
	eventQueue->head = 0;
	eventQueue->tail = 0;
	eventQueue->full = false;
	eventQueue->empty = true;
}

/**
 * @brief  initHSM 
 *         Initializes HSM
 * @param  hsm: pointer to HSM to initialize
 * @param  initialState: pointer to initial state
 * @param  eventQueue: pointer to event queue to be assigned to HSM
 * @retval None
 */
void initHSM( hsm_t * hsm, const state_t * initialState, eventQueue_t * eventQueue ){
	hsm->currentState = initialState;
	hsm->eventQueue = eventQueue;
	hsm->currentState->handler( HSM_INITIAL );
}

/**
 * @brief  setState
 *         Sets state of HSM.
 * @param  hsm: pointer to HSM
 * @param  state: state to set
 * @retval None
 */
void setState( hsm_t * hsm, const state_t * state ){
	// Check, whether it is a self transition
	if(hsm->currentState == state){
		state->handler( HSM_ON_EXIT );
		state->handler( HSM_ON_ENTRY );
		sendEvent( hsm, HSM_INITIAL );
	}
	else{
	  // Go to root state from current
	  uint8_t curr_path_cntr = 0;
    
	  up_path[curr_path_cntr] = hsm->currentState;
	  while( up_path[curr_path_cntr] != up_path[curr_path_cntr]->parent ){
	  	curr_path_cntr++;
	  	up_path[curr_path_cntr] = up_path[curr_path_cntr-1]->parent;
	  }
	  // Go to root state from destination
	  uint8_t dest_path_cntr = 0;
    
	  down_path[dest_path_cntr] = state;
	  while( down_path[dest_path_cntr] != down_path[dest_path_cntr]->parent ){
	  	dest_path_cntr++;
	  	down_path[dest_path_cntr] = down_path[dest_path_cntr-1]->parent;
	  }	
	  // Compare and prune common ancestors
	  bool no_entry = false;
	  bool no_exit = false;
	  while( up_path[curr_path_cntr] == down_path[dest_path_cntr] ){
	  	if( curr_path_cntr == 0 ){
	  		no_exit = true;
	  		dest_path_cntr--;
	  		break;
	  	}
	  	else if( dest_path_cntr == 0 ){
	  		no_entry = true;
	  	  curr_path_cntr--;  
	  		break;
	  	}
	  	else {
	  		curr_path_cntr--;  
	  		dest_path_cntr--;
	  	}
	  }
	  // Execute all on_exit actions for up path, if there are
	  uint8_t i = 0;
	  if( no_exit == false){
	    while( i <= curr_path_cntr ){
	  	  up_path[i]->handler( HSM_ON_EXIT );
	  	  i--;
	    }
	  }
	  // Execute all on_entry actions for down path
	  if( no_entry == false ){
	    i = dest_path_cntr + 1;
	    do{
    		i--;
	    	down_path[i]->handler( HSM_ON_ENTRY );
	    } while( i != 0 );
	  }
	  // Set new state
	  hsm->currentState = state;
	  sendEvent( hsm, HSM_INITIAL );
  }
}

/**
 * @brief  processHSM
 *         Processes HSM's events in queue.          
 * @param  hsm: pointer to HSM
 * @retval None
 */
void processHSM( hsm_t * hsm){
	event_t currentEvent;
	while(getEvent( hsm->eventQueue, &currentEvent )!= QUEUE_EMPTY){
    (hsm->currentState->handler)(currentEvent);
	}
}

/**
 * @brief  sendEvent
 *         Sends event to HSM's event queue
 * @param  hsm: pointer to HSM
 * @param  event: event to send
 * @retval None
 */
void sendEvent( hsm_t * hsm, event_t event ){
	// Disabling interrupts for atomic execution
	__disable_irq();
    putEvent(hsm->eventQueue, event);
	__enable_irq();
}

/* Private functions */

/**
 * @brief  getEvent
 *         Gets event from event queue
 * @param  eventQueue: pointer to event queue
 * @param  event: pointer to buffer for storing received first event from queue
 * @retval Returns QUEUE_OK if succeeded, and QUEUE_EMPTY if queue empty and there are no events
 */
static eventQueueError_t getEvent( eventQueue_t * eventQueue, event_t * event ){
	if(!eventQueue->empty){
		*event = eventQueue->queue[eventQueue->tail];
		eventQueue->full = false;
		eventQueue->tail = (eventQueue->tail < (eventQueue->length - 1)) ? eventQueue->tail + 1 : 0;
		eventQueue->empty = (eventQueue->tail == eventQueue->head) ? true : false;
		return QUEUE_OK;
	} 
	else {
		return QUEUE_EMPTY;
	}
}

/**
 * @brief  getEvent
 *         Gets event from event queue
 * @param  eventQueue: pointer to event queue
 * @param  event: event to put in queue
 * @retval Returns QUEUE_OK if succeeded, and QUEUE_FULL if queue full and event cannot be inserted
 */
static eventQueueError_t putEvent( eventQueue_t * eventQueue, event_t event ){
	if(!eventQueue->full)	{
		eventQueue->queue[eventQueue->head] = event;
		eventQueue->empty = false;
	  eventQueue->head = (eventQueue->head < (eventQueue->length - 1)) ? eventQueue->head + 1 : 0;
		eventQueue->full = (eventQueue->tail == eventQueue->head) ? true : false;
		return QUEUE_OK;
	}
	else {
		return QUEUE_FULL;		
	}
}
