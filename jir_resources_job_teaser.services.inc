<?php
/**
* @file
* Actual Resource
*/

function job_teaser_resource() {
    $api = array(
        'jir_mobile_resource_job' => array(
            'operations' => array(
                'retrieve' => array(
                    'help' => 'Retrieve JIR mobile Job resource.',
                    'file' => array(
                        'type' => 'inc',
                        'module' => 'jir_mobile_resources',
                        'name' => 'includes/jir_resources_job_teaser.services'
                    ),
                    'callback' => 'job_teaser_resource_retrieve',
                    'access callback' => 'user_access',
                    'access arguments' => array('access content'),
                    'access arguments append' => FALSE,
                    'args' => array(
                        'name' => 'nid',
                        'type' => 'int',
                        'description' => 'Function to perform',
                        'source' => array(
                            'path' => '0'
                        ),
                        'optional' => TRUE,
                        'default' => '0'
                    )
                )
            )
        )
    );

    return $api;
}

/**
 * @param $nid
 *      The nid of the job node
 * @return array
 *      The job node processed data
 */
function job_teaser_resource_retrieve($nid) {
    $node = node_load($nid);
    if ($node->type == 'job') {
        $employer_id = $node->field_employer[LANGUAGE_NONE][0]['value'];
        $employer = node_load($employer_id);
    }
    return array();
}