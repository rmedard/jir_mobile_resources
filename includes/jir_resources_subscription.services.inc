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
                            'type' => 'array',
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
        $data = (object) $data;
        if (!isset($data->names) || !isset($data->email)) {
            watchdog('RestApi', 'Missing names or email. Please correct', null, WATCHDOG_ERROR);
            throw services_error('Missing names or email. Please correct', 406);
        } elseif (simplenews_user_is_subscribed($data->email, 104)) {
            watchdog('RestApi', t('User [@email] already subscribed', array('@email' => $data->email)), null, WATCHDOG_ERROR);
            throw services_error(t('You are already subscribed', array('@email' => $data->email)), 406);
        } else {
            $subscribed = simplenews_subscribe_user($data->email, 104, false, 'website');
            if ($subscribed) {
                watchdog('RestApi', 'User subscribed successfully: ' . $data->email, null, WATCHDOG_INFO);
                _simplenews_realname_set_realname($data->email, $data->names);
            } else {
                throw services_error('User subscription failed', 400);
            }
            return array('message' => 'User subscribed successfully', 'code' => 201);
        }
    } catch (ServicesException $e) {
        return array('message' => $e->getMessage(), 'code' => $e->getCode());
    }
}