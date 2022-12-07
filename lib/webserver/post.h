#ifndef _POST_H_
#define _POST_H_

/**
 * When post request received to trigger endpoint, this function called.
 * 
 * @param pname Triggered payload name. (Ex. payloadname.txt)
 */
void post_trigger_cb(char *pname);

/**
 * When post request received to upload endpoint, this function called.
 * 
 * @param upname Uploaded file name. (Ex. uploadedpayload.txt)
 */
void _post_upload_cb(char *upname);

/**
 * When post request received to settings endpoint, this function called.
 * 
 */
void post_settings_cb();

#endif
