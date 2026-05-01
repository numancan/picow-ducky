#ifndef _POST_H_
#define _POST_H_

/**
 * When post request received to TRIGGER endpoint, this function called.
 * 
 * @param pname Triggered payload name. (Ex. payloadname.txt)
 */
void post_trigger_cb();

/**
 * When post request received to UPLOAD endpoint, this function called.
 * 
 * @param upname Uploaded file name. (Ex. uploadedpayload.txt)
 */
void post_upload_cb(char *upname);

/**
 * When post request received to SETTINGS endpoint, this function called.
 * 
 */
void post_settings_cb();

#endif
