<?php
/**
 * @file
 * Actual Resource
 */

function job_teaser_resource() {
    watchdog('jirapi', 'Job teaser resource entered');
    $api = array(
      'jir_job' => array(
        'operations' => array(
          'retrieve' => array(
            'help' => 'Retrieve JIR mobile Job resource.',
            'file' => array(
              'type' => 'inc',
              'module' => 'jir_mobile_resources',
              'name' => 'includes/jir_resources_job_teaser.services',
            ),
            'callback' => 'job_teaser_resource_retrieve',
            'access callback' => 'user_access',
            'access arguments' => array('access content'),
            'args' => array(
              array(
                'name' => 'nid',
                'type' => 'int',
                'description' => 'Function to perform',
                'optional' => FALSE,
                'source' => array('path' => '0'),
              ),
            ),
          ),
        ),
      ),
    );
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
          'posting_type' => $node->field_posting_type[LANGUAGE_NONE][0]['tid'],
          'category' => array_values($node->field_category[LANGUAGE_NONE]),
          'deadline' => $node->field_deadline_for_application[LANGUAGE_NONE][0]['value'],
          'additional_email_cv' => $node->field_additional_email_where_to[LANGUAGE_NONE][0]['email'],
          'number_of_positions' => $node->field_number_of_positions[LANGUAGE_NONE][0]['value'],
          'contract_type' => $node->field_contrat_type[LANGUAGE_NONE][0]['tid'],
          'desired_experience' => $node->field_desired_experience[LANGUAGE_NONE][0]['tid'],
          'desired_education_level' => $node->field_desired_education_level[LANGUAGE_NONE][0]['tid'],
          'offer_type' => $node->field_offer_type[LANGUAGE_NONE][0]['value'],
          'city' => $node->field_job_city[LANGUAGE_NONE][0]['safe_value'],
          'country' => $node->field_job_country[LANGUAGE_NONE][0]['iso2'],
          'action_apply_default_url' => $node->field_apply_default[LANGUAGE_NONE][0]['url'],
          'application_form_type' => $node->field_application_form_type[LANGUAGE_NONE][0]['tid'],
        );
    }
    return $job;
}