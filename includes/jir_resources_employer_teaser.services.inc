<?php
/**
 * @file
 * Actual Resource
 */

function employer_teaser_resource() {

    $api_employer = array(
        'jir_employer' => array(
            'operations' => array(
                'retrieve' => array(
                    'help' => 'Retrieve JIR mobile Employer resource.',
                    'file' => array(
                        'type' => 'inc',
                        'module' => 'jir_mobile_resources',
                        'name' => 'includes/jir_resources_employer_teaser.services',
                    ),
                    'callback' => 'employer_teaser_resource_retrieve',
                    'access callback' => 'user_is_logged_in',
                    //'access arguments' => array('access content'),
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
                'index' => array(
                    'help' => 'Retrieve a list od JIR mobile Employer resource',
                    'file' => array(
                        'type' => 'inc',
                        'module' => 'jir_mobile_resources',
                        'name' => 'includes/jir_resources_employer_teaser.services',
                    ),
                    'callback' => 'employer_teasers_list',
                    'access callback' => 'user_is_logged_in',
                    'args' => array(
                        array(
                            'name' => 'page',
                            'type' => 'int',
                            'description' => 'Page number',
                            'source' => array(
                                'param' => 'page',
                            ),
                            'optional' => TRUE,
                            'default value' => 0,
                        ),
                        array(
                            'name' => 'pageSize',
                            'type' => 'int',
                            'description' => 'Page size',
                            'source' => array(
                                'param' => 'pageSize',
                            ),
                            'optional' => TRUE,
                            'default value' => 10,
                        ),
                        array(
                            'name' => 'search',
                            'type' => 'string',
                            'description' => 'Search string',
                            'source' => array(
                                'param' => 'search',
                            ),
                            'optional' => TRUE,
                            'default value' => '',
                        ),
                    ),
                ),
            ),
        ),
    );
    return $api_employer;
}

function employer_teaser_resource_retrieve () {

}

function employer_teasers_list ($page, $pageSize) {
    $employers = array();
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'employer')
        ->propertyCondition('status', NODE_PUBLISHED)
        ->range($page * $pageSize, $pageSize);

    $result = $query->execute();
    if (isset($result['node'])) {
        $employer_ids = array_keys($result['node']);
        foreach (entity_load('node', $employer_ids) as $employer) {
            array_push($employers, map_employer_teaser($employer));
        }
    }
    return $employers;
}

function map_employer_teaser ($node) {
    return array(
        'title' => $node->title,
        'logo_url' => isset($employer->field_employer_logo[LANGUAGE_NONE][0]['uri']) ?
            $node->field_employer_logo[LANGUAGE_NONE][0]['uri'] : ''
    );
}