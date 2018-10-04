<?php
/**
 * @file
 * Actual Resource
 */

function job_teaser_resource() {
    $api = [
      'jir_mobile_resource_job' => [
        'operations' => [
          'retrieve' => [
            'help' => 'Retrieve JIR mobile Job resource.',
            'file' => [
              'type' => 'inc',
              'module' => 'jir_mobile_resources',
              'name' => 'includes/jir_resources_job_teaser.services',
            ],
            'callback' => 'job_teaser_resource_retrieve',
            'access callback' => 'user_access',
            'access arguments' => ['access content'],
            'access arguments append' => FALSE,
            'args' => [
              'name' => 'nid',
              'type' => 'int',
              'description' => 'Function to perform',
              'optional' => FALSE,
              'source' => [
                'param' => 'nid',
              ]
            ],
          ],
        ],
      ],
    ];

    return $api;
}

/**
 * @param $nid
 *      The nid of the job node
 *
 * @return array
 *      The job node processed data
 */
function job_teaser_resource_retrieve($nid) {
    $node = node_load($nid);
    $job = array();
    if ($node->type == 'job') {
        //        $employer_id = $node->field_employer[LANGUAGE_NONE][0]['value'];
        //        $employer = node_load($employer_id);

        $job = array(
          'job_title' => $node->title,
          'job_id' => $node->nid,
          'created_date' => $node->created,
          'contact_name' => $node->field_contact_name[LANGUAGE_NONE][0]['value'],
        );
    }
    return $job;
}