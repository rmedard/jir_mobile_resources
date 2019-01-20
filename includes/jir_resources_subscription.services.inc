<?php
/**
 * @file
 * Actual Resource
 */

function subscription_resource()
{
    return array(
        'jir_subscription' => array(
            'operations' => array(
                'create' => array(
                    'help' => 'Creates a user subscription to JIR newsletter',
                    'file' => array(
                        'type' => 'inc',
                        'module' => 'jir_mobile_resources',
                        'name' => 'includes/jir_resources_subscription.services'
                    ),
                    'callback' => '_subscription_resource_create',
                    'access callback' => 'user_is_logged_in',
//                    'access arguments' => array('note resource create'),
                    'access arguments append' => FALSE,
                    'args' => array(
                        array(
                            'name' => 'data',
                            'type' => 'struct',
                            'description' => 'The resource object',
                            'source' => 'data',
                            'optional' => FALSE,
                        ),
                    ),
                )
            )
        )
    );
}

function _subscription_resource_create($data)
{
    try {
        watchdog('RestApi: sent data', json_encode($data));
        if (!isset($data->names) || !isset($data->email)) {
            return services_error('Missing names or email. Please correct', 406);
        } elseif (simplenews_user_is_subscribed($data->email, 104)) {
            watchdog('RestApi', 'User already subscribed: ' . $data->email, null, WATCHDOG_INFO);
            return services_error('You are already subscribed', 200);
        } else {
            $subscribed = simplenews_subscribe_user($data->email, 104, false, 'website');
            if ($subscribed) {
                watchdog('RestApi', 'User subscribed successfully: ' . $data->email, null, WATCHDOG_INFO);
                _simplenews_realname_set_realname($data->email, $data->names);
                return array('message' => 'User subscribed successfully');
            } else {
                watchdog('RestApi', 'User subscription failed');
                return services_error('User subscription failed', 400);
            }
        }
    } catch (ServicesException $e) {
        watchdog('RestApi', 'Services failure: ' . $e, null, WATCHDOG_ERROR);
        return array('message' => 'User subscription failed');
    }
}