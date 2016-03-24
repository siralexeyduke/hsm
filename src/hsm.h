/**
  * @file    hsm.h
  * @author  Alexey Serdyuk
  * @version V0.1.0
  * @date    15-July-2015
  * @brief   Hierarchical state machine header file.
  */
#ifndef HSM_H
#define HSM_H

#include <stdbool.h>
#include <stdint.h>

/** Defines maximum number of events in system */
#define MAX_EVENT 255

/**
 * Define HSM state. Macro creates state handler and redefines state_t __STATE__ initializing it
 * with handler [__STATE__]_handler and parent pointer & __PARENT__. \n
 * __STATE__ - const state_t entity, existing state \n
 * __PARENT__ - const state_t entity, existing state which is used as a parent state. \n 
 * @note If it is super state __PARENT__ have to be equal to __STATE__.
 */
#define HSM_STATE_ACTION( __STATE__ , __PARENT__) static void __STATE__ ## _handler ( event_t event );            \
                                                         state_t __STATE__ = { .handler = __STATE__ ## _handler , \
                                                                               .parent  = & __PARENT__,           \
                                                   														 .history = &__STATE__ };         \
                                                   static void __STATE__ ## _handler ( event_t event )
																										 
/** System events enumeration */
enum hsm_sys_event_enum { 
  HSM_INITIAL = 0,          /**< Internal event. */
  HSM_ON_ENTRY,             /**< Internal event. */
  HSM_ON_EXIT,	            /**< Internal event. */
	HSM_USER_START
};

/** Event type. Represents abstract events. */
typedef uint16_t event_t;

/** Error codes enumeration for event queue methods*/
typedef enum eventQueueError_enum{
	QUEUE_OK,   /**< No error */
	QUEUE_FULL, /**< Queue is full */
	QUEUE_EMPTY /**< Queue is empty */
}	eventQueueError_t;
							 
/** Event queue type */
typedef struct eventQueue_struct{
	event_t * queue; /**< Buffer of events (event_t instancies) */
	uint8_t length;  /**< Length of event queue */
	uint8_t head;    /**< Pointer to head of event queue circular buffer */
	uint8_t tail;    /**< Pointer to tail of event queue circular buffer */
	bool full;       /**< Event queue full flag */
	bool empty;      /**< Event queue empty flag */
} eventQueue_t;

/** State handler pointer type */
typedef void (* const stateHandler_t)( event_t );

/** State type */
typedef struct state_struct{
  stateHandler_t              handler;
	const struct state_struct * const parent;
	const struct state_struct * history;
} state_t;

/** Hierarchical state machine type */
typedef struct hsm_struct{
	const state_t * currentState;
	eventQueue_t * eventQueue;
} hsm_t;

/* Public functions */
void initEventQueue( eventQueue_t * eventQueue, event_t * queue, uint8_t length );
void initHSM( hsm_t * hsm, const state_t * initialState, eventQueue_t * eventQueue );
void setState( hsm_t * hsm, const state_t * state);
void processHSM( hsm_t * hsm );
void sendEvent( hsm_t * hsm, event_t event );

#endif /* HSM_H */
