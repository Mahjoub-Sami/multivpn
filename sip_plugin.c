
/* 

 */



/* MULTIVPN HEADERS*/
#include <pthread.h>
#include <unistd.h>
#include "generictunel.h"
#include "sip_plugin.h"
#include "tuntap.h"
#include "debug.h"
#include "base64.h"


/* PJSIP HEADERS */
#include <pjsip.h>
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include <pjsip_ua.h>
#include <pjsip_simple.h>
#include <pjlib-util.h>
#include <pjlib.h>

/* For logging purpose. */
#define THIS_FILE   "simpleua.c"

// #include "util.h"


/* Settings */
#define AF		pj_AF_INET() /* Change to pj_AF_INET6() for IPv6.
				      * PJ_HAS_IPV6 must be enabled and
				      * your system must support IPv6.  */

#if 0
#define SIP_PORT	5080	     /* Listening SIP port		*/
#define RTP_PORT	5000	     /* RTP port			*/
#else
#define SIP_PORT	5060	     /* Listening SIP port		*/
#define RTP_PORT	4000	     /* RTP port			*/
#endif

#define MAX_MEDIA_CNT	2	     /* Media count, set to 1 for audio
				      * only or 2 for audio and video	*/


static pthread_t sip_plugin_event_thread;
/*
 * Static variables.
 */

static pj_bool_t	     g_complete;    /* Quit flag.		*/
static pjsip_endpoint	    *g_endpt;	    /* SIP endpoint.		*/
static pj_caching_pool	     cp;	    /* Global pool factory.	*/
//static pjmedia_endpt	    *g_med_endpt;   /* Media endpoint.		*/
#if 0
static pjmedia_transport_info g_med_tpinfo[MAX_MEDIA_CNT]; /* Socket info for media	*/
static pjmedia_transport    *g_med_transport[MAX_MEDIA_CNT];/* Media stream transport	*/
#endif
//static pjmedia_sock_info     g_sock_info[MAX_MEDIA_CNT]; /* Socket info array	*/

/* Call variables: */
//static pjsip_inv_session    *g_inv;	    /* Current invite session.	*/
#if 0
static pjmedia_stream       *g_med_stream;  /* Call's audio stream.	*/
static pjmedia_snd_port	    *g_snd_port;    /* Sound device.		*/

#if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)
static pjmedia_vid_stream   *g_med_vstream; /* Call's video stream.	*/
static pjmedia_vid_port	    *g_vid_capturer;/* Call's video capturer.	*/
static pjmedia_vid_port	    *g_vid_renderer;/* Call's video renderer.	*/
#endif	/* PJMEDIA_HAS_VIDEO */
#endif

/*
 * Prototypes:
 */

/* Callback to be called when SDP negotiation is done in the call: */
//static void call_on_media_update( pjsip_inv_session *inv, pj_status_t status);

/* Callback to be called when invite session's state has changed: */
static void call_on_state_changed( pjsip_inv_session *inv,  pjsip_event *e);

/* Callback to be called when dialog has forked: */
static void call_on_forked(pjsip_inv_session *inv, pjsip_event *e);

/* Callback to be called to handle incoming requests outside dialogs: */
static pj_bool_t on_rx_request( pjsip_rx_data *rdata );




/* This is a PJSIP module to be registered by application to handle
 * incoming requests outside any dialogs/transactions. The main purpose
 * here is to handle incoming INVITE request message, where we will
 * create a dialog and INVITE session for it.
 */
static pjsip_module mod_simpleua =
{
    NULL, NULL,			    /* prev, next.		*/
    { "mod-simpleua", 12 },	    /* Name.			*/
    -1,				    /* Id			*/
    PJSIP_MOD_PRIORITY_APPLICATION, /* Priority			*/
    NULL,			    /* load()			*/
    NULL,			    /* start()			*/
    NULL,			    /* stop()			*/
    NULL,			    /* unload()			*/
    &on_rx_request,		    /* on_rx_request()		*/
    NULL,			    /* on_rx_response()		*/
    NULL,			    /* on_tx_request.		*/
    NULL,			    /* on_tx_response()		*/
    NULL,			    /* on_tsx_state()		*/
};



void app_perror(char *file,char *message,int status)
{
}

/* Notification on incoming messages */
static pj_bool_t logging_on_rx_msg(pjsip_rx_data *rdata)
{
    PJ_LOG(4,(THIS_FILE, "RX %d bytes %s from %s %s:%d:\n"
			 "%.*s\n"
			 "--end msg--",
			 rdata->msg_info.len,
			 pjsip_rx_data_get_info(rdata),
			 rdata->tp_info.transport->type_name,
			 rdata->pkt_info.src_name,
			 rdata->pkt_info.src_port,
			 (int)rdata->msg_info.len,
			 rdata->msg_info.msg_buf));
    
    /* Always return false, otherwise messages will not get processed! */
    return PJ_FALSE;
}

/* Notification on outgoing messages */
static pj_status_t logging_on_tx_msg(pjsip_tx_data *tdata)
{
    
    /* Important note:
     *	tp_info field is only valid after outgoing messages has passed
     *	transport layer. So don't try to access tp_info when the module
     *	has lower priority than transport layer.
     */

    PJ_LOG(4,(THIS_FILE, "TX %d bytes %s to %s %s:%d:\n"
			 "%.*s\n"
			 "--end msg--",
			 (tdata->buf.cur - tdata->buf.start),
			 pjsip_tx_data_get_info(tdata),
			 tdata->tp_info.transport->type_name,
			 tdata->tp_info.dst_name,
			 tdata->tp_info.dst_port,
			 (int)(tdata->buf.cur - tdata->buf.start),
			 tdata->buf.start));

    /* Always return success, otherwise message will not get sent! */
    return PJ_SUCCESS;
}

/* The module instance. */
static pjsip_module msg_logger = 
{
    NULL, NULL,				/* prev, next.		*/
    { "mod-msg-log", 13 },		/* Name.		*/
    -1,					/* Id			*/
    PJSIP_MOD_PRIORITY_TRANSPORT_LAYER-1,/* Priority	        */
    NULL,				/* load()		*/
    NULL,				/* start()		*/
    NULL,				/* stop()		*/
    NULL,				/* unload()		*/
    &logging_on_rx_msg,			/* on_rx_request()	*/
    &logging_on_rx_msg,			/* on_rx_response()	*/
    &logging_on_tx_msg,			/* on_tx_request.	*/
    &logging_on_tx_msg,			/* on_tx_response()	*/
    NULL,				/* on_tsx_state()	*/

};

int sip_start(int argc,char **argv)
{
    //pj_pool_t *pool = NULL;
    pj_status_t status;
//    unsigned i;

    /* Must init PJLIB first: */
    status = pj_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    pj_log_set_level(5);

    /* Then init PJLIB-UTIL: */
    status = pjlib_util_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /* Must create a pool factory before we can allocate any memory. */
    pj_caching_pool_init(&cp, &pj_pool_factory_default_policy, 0);

    /* Create global endpoint: */
    {
	const pj_str_t *hostname;
	const char *endpt_name;

	/* Endpoint MUST be assigned a globally unique name.
	 * The name will be used as the hostname in Warning header.
	 */

	/* For this implementation, we'll use hostname for simplicity */
	hostname = pj_gethostname();
	endpt_name = hostname->ptr;

	/* Create the endpoint: */

	status = pjsip_endpt_create(&cp.factory, endpt_name, 
				    &g_endpt);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    }


    /* 
     * Add UDP transport, with hard-coded port 
     * Alternatively, application can use pjsip_udp_transport_attach() to
     * start UDP transport, if it already has an UDP socket (e.g. after it
     * resolves the address with STUN).
     */
    {
	pj_sockaddr addr;

	pj_sockaddr_init(AF, &addr, NULL, (pj_uint16_t)SIP_PORT);
	
//	if (AF == pj_AF_INET()) {
	    status = pjsip_udp_transport_start( g_endpt, &addr.ipv4, NULL, 
						1, NULL);
//	} else if (AF == pj_AF_INET6()) {
//	    status = pjsip_udp_transport_start6(g_endpt, &addr.ipv6, NULL,
//						1, NULL);
//	} else {
//	    status = PJ_EAFNOTSUP;
//	}

	if (status != PJ_SUCCESS) {
	    app_perror(THIS_FILE, "Unable to start UDP transport", status);
	    return 1;
	}
    }


    /* 
     * Init transaction layer.
     * This will create/initialize transaction hash tables etc.
     */
    status = pjsip_tsx_layer_init_module(g_endpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /* 
     * Initialize UA layer module.
     * This will create/initialize dialog hash tables etc.
     */
    status = pjsip_ua_init_module( g_endpt, NULL );
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /* 
     * Init invite session module.
     * The invite session module initialization takes additional argument,
     * i.e. a structure containing callbacks to be called on specific
     * occurence of events.
     *
     * The on_state_changed and on_new_session callbacks are mandatory.
     * Application must supply the callback function.
     *
     * We use on_media_update() callback in this application to start
     * media transmission.
     */
    {
	pjsip_inv_callback inv_cb;

	/* Init the callback for INVITE session: */
	pj_bzero(&inv_cb, sizeof(inv_cb));
	inv_cb.on_state_changed = &call_on_state_changed;
	inv_cb.on_new_session = &call_on_forked;
//	inv_cb.on_media_update = &call_on_media_update;

	/* Initialize invite session module:  */
	status = pjsip_inv_usage_init(g_endpt, &inv_cb);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    }

    /* Initialize 100rel support */
    status = pjsip_100rel_init_module(g_endpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, status);

    /*
     * Register our module to receive incoming requests.
     */
    status = pjsip_endpt_register_module( g_endpt, &mod_simpleua);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /*
     * Register message logger module.
     */
    status = pjsip_endpt_register_module( g_endpt, &msg_logger);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

   


	debug(1,"SIP Plugin launching thread for looping events");
	
	pthread_create( &sip_plugin_event_thread, NULL, sip_loop_sip_events, NULL);
	
    
	/*
		Aqui es donde estamos esperando a recibir datos del PIPE (que viene del TUN DRIVER)
		hacemos I/O BLOCK con select
	*/
	sip_loop_tun_events();
	
	// Aqui llegamos cuando ha muerto todo ;)

    return 0;
}



/*
 * Callback when INVITE session state has changed.
 * This callback is registered when the invite session module is initialized.
 * We mostly want to know when the invite session has been disconnected,
 * so that we can quit the application.
 */
static void call_on_state_changed( pjsip_inv_session *inv, 
				   pjsip_event *e)
{
    PJ_UNUSED_ARG(e);

    if (inv->state == PJSIP_INV_STATE_DISCONNECTED) {

	PJ_LOG(3,(THIS_FILE, "Call DISCONNECTED [reason=%d (%s)]", 
		  inv->cause,
		  pjsip_get_status_text(inv->cause)->ptr));

	PJ_LOG(3,(THIS_FILE, "One call completed, application quitting..."));
	g_complete = 1;

    } else {

	PJ_LOG(3,(THIS_FILE, "Call state changed to %s", 
		  pjsip_inv_state_name(inv->state)));

    }
}


/* This callback is called when dialog has forked. */
static void call_on_forked(pjsip_inv_session *inv, pjsip_event *e)
{
    /* To be done... */
    PJ_UNUSED_ARG(inv);
    PJ_UNUSED_ARG(e);
}


/*
 * Callback when incoming requests outside any transactions and any
 * dialogs are received. We're only interested to hande incoming INVITE
 * request, and we'll reject any other requests with 500 response.
 */
static pj_bool_t on_rx_request( pjsip_rx_data *rdata )
{
//    pj_sockaddr hostaddr;
//    char temp[80], hostip[PJ_INET6_ADDRSTRLEN];
//    pj_str_t local_uri;
//    pjsip_dialog *dlg;
//    pjmedia_sdp_session *local_sdp;
//    pjsip_tx_data *tdata;
//    unsigned options = 0;
//    pj_status_t status;

	if (  (rdata->msg_info.msg->line.req.method.id != PJSIP_OPTIONS_METHOD))
		//&& (rdata->msg_info.msg->line.req.method.id != PJSIP_INFO_METHOD))
	{
		debug(2,"Received no INFO/OPTIONS Request, reply 500 sent");
		pj_str_t reason = pj_str("Go home");
		pjsip_endpt_respond_stateless( g_endpt, rdata,500, &reason,NULL, NULL);
	}
	debug(1,"Received INFO/OPTIONS Request, lets parse it");
	
	pjsip_cid_hdr *headerMultivpn;
	pj_str_t hdrname = { .ptr = "Multivpn", .slen = 8 };
	
	headerMultivpn = pjsip_msg_find_hdr_by_name(rdata->msg_info.msg ,&hdrname,NULL);
	
	if (!headerMultivpn)
	{
		debug(2,"No multivpn header found! Nothing to parse");
		pj_str_t reason = pj_str("Go home and sleep");
		pjsip_endpt_respond_stateless( g_endpt, rdata,500, &reason,NULL, NULL);
		return PJ_TRUE;
	}	
	else debug(1,"Header multivpn found ! Lets play with it");
	

    char payload[2048];
    memset(payload, 0, sizeof(payload));
    strncpy(payload, headerMultivpn->id.ptr, headerMultivpn->id.slen);
	
	debug(1,"Received payload is: %s with len %d",payload,strlen(payload));
	
	
	unsigned char *decodedpayload;
	size_t lenDecoded;
	decodedpayload = base64_decode(payload,strlen(payload),&lenDecoded);
	debug(1,"Decoded %d bytes",lenDecoded);
	debug(1,"Decoded payload with len %d and value %.3s",lenDecoded, decodedpayload);
// unsigned char *base64_decode(const char *data, size_t input_length, size_t *output_length);
	
	debug(1,"Now sending to PIPE for TUNDRIVER !");

	int nBytes = lenDecoded;
	
	nBytes=write(global_v.pipe_from_plugin[1],decodedpayload,nBytes);
	if (nBytes<=0)
		debug(3,"SIP Plugin: Failed Writing to Pipe");
	else    debug(3,"SIP Plugin: Write %d bytes to pipe",nBytes);
                
	// Tras el parsing, contestamos:
	pj_str_t reason = pj_str("Traffic Accepted");
   pjsip_endpt_respond_stateless( g_endpt, rdata,666, &reason,NULL, NULL);
   
   debug(1,"End process of request");
	return PJ_TRUE;



}

 
int sip_envia_datos(unsigned char *datos,int longitud)
{
	debug(1,"Starting function to send %d bytes", longitud);
	
	debug(1,"Now base64 encoding %d bytes",longitud);

	int longitudEncodeado;                    
	char * payload;
	payload = base64_encode(datos,longitud,(size_t *)&longitudEncodeado);
	                    	
	pj_status_t status;
	pjsip_tx_data *tdata;
	pj_str_t target = pj_str(global_v.sip_remoteuri);
	pj_str_t from = pj_str(global_v.sip_fromuri);

	status = pjsip_endpt_create_request(
			g_endpt,
			&pjsip_options_method,
			&target,
			&from,
			&target,
			NULL,
			NULL,
			-1,
			NULL,
			&tdata
	);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

	pjsip_transaction *tsx;
	status = pjsip_tsx_create_uac(NULL, tdata, &tsx);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

	// Create Custom header
	pj_str_t hdr_name = { .ptr = "Multivpn", .slen = 8 };
	pj_str_t hdr_value = { .ptr = payload, .slen = longitudEncodeado };
	pjsip_generic_string_hdr *multivpn_hdr = pjsip_generic_string_hdr_create(
			tdata->pool,
			&hdr_name,
			&hdr_value
	);

	// Add to message
	pjsip_msg_add_hdr(tdata->msg, (pjsip_hdr *) multivpn_hdr);


	status = pjsip_tsx_send_msg(tsx, NULL);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

	return 0;
}



void* sip_loop_sip_events(void *none)
{
    for (;!g_complete;) {
			//pj_time_val timeout = {0, 10};
			//pjsip_endpt_handle_events(g_endpt, &timeout);
			pjsip_endpt_handle_events(g_endpt, NULL);
			debug(1,"Looping SIP");
    }

    return NULL;
 }

void sip_loop_tun_events()
{
	 unsigned char buffer[BLOCK_SIZE];
    int nBytes;
    fd_set  setReading;
    int maxfd;
    
    debug(2,"SIP Plugin: Starting LOOPING for events from TUN");
     maxfd = global_v.pipe_to_plugin[0]+1;
    while (1)
    {
        FD_SET(global_v.pipe_to_plugin[0],&setReading);
        debug(3,"SIP Plugin: Blocking now, until data from tundriver is received");
        
        select(maxfd,&setReading,NULL,NULL,NULL);

			debug(1,"SIP PLUGIN: Select unlocked !");
			        
        if (FD_ISSET(global_v.pipe_to_plugin[0],&setReading))
        {
            // Message is from pipe
            nBytes=read(global_v.pipe_to_plugin[0],buffer, BLOCK_SIZE-1);
            if (nBytes<=0)
                debug(3,"SIP Plugin: failed reading From Pipe");
            else    
            {
                debug(3,"SIP Plugin: Read %d bytes from pipe",nBytes);
         		 sip_envia_datos(buffer,nBytes);
            
        		}
        
    		}
    }
 }

/*
	MULTIVPN
	
*/

// FIXME:
int sip_checkParameters()
{
	
	debug(1,"SIP Plugin Checking parameters...");
	/*
		Necesitamos como poco:
		sipport
		siptransport
		sipremoteuri
		tunelip
		
	*/
	if (!global_v.sip_port)
	{
		debug(2,"Missing sip port in config file");
		return (-1);
	}
	if (!global_v.sip_transport)
	{
		debug(2,"Missing sip transport in config file");
		return (-1);
	}
	if (!global_v.sip_remoteuri)
	{
		debug(2,"Missing sip remoteuri in config file");
		return (-1);
	}
		if (!global_v.sip_port)
	{
		debug(2,"Missing sip port in config file");
		return (-1);
	}
		
	debug(1,"Everything is OK on SIP Parameters, let's continue");
	return 0;
}

// FIXME:
int sip_build()
{
	debug(1,"SIP Plugin: Building");
	if (tun_open())
	{
		debug_error("Tun_open failed");
		return -1;
	}
	else debug(3,"SIP_PLUGIN Succesfull open tun");
	
	if (	tun_UP() )
	{
		debug_error("TUN UP Failed");
		return -1;
	}
	else debug(3,"SIP_PUGIN TUN UP is OK");
	
	tun_setIP(global_v.local_ip);
	tun_setNETMASK(global_v.local_netmask);
	
	return (0);
	
}


int sip_getTunTap()
{
	return 0;
	//return (tun_getFile());

}
void *sip_plugin_getStart()
{
	return sip_start;
}
void  *sip_plugin_getCheckParameters()
{
	debug(1,"Returning pointer to function sip plugin getCheckParameters");
	return sip_checkParameters;
}
void *sip_plugin_getFillFDSET()
{
	debug_error("To be implemented getFillFDSET");
	return 0;
	//return sip_fillFDSET;
}
void *sip_plugin_getCheckFDISSET()
{
	debug_error("To be implemented FDISSET");
	return 0;
	//return sip_checkFDISSET;
}
void  *sip_plugin_getTunTap()
{
	debug_error("To be implemented getTUNTAP");
	return 0;
	//return sip_getTunTap;
}
void *sip_plugin_getBuild()
{
	return sip_build;
}