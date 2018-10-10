<?php
/**
 * @file
 * Actual Resource
 */

function job_teaser_resource()
{
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
                    'access callback' => 'user_is_logged_in',
                    //                    'access arguments' => array('access content'),
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
                    'help' => 'Retrieve a lit od JIR mobile Job resource',
                    'file' => array(
                        'type' => 'inc',
                        'module' => 'jir_mobile_resources',
                        'name' => 'includes/jir_resources_job_teaser.services',
                    ),
                    'callback' => 'job_teasers_list',
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
                        )
                    ),
                )
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
function job_teaser_resource_retrieve($nid)
{
    $node = node_load($nid);
    $job = array();
    if ($node and is_object($node)) {
        if ($node->type == 'job') {
            $job = get_job_resource($node);
        }
    }
    return $job;
}

/**
 * @return array
 *      Return a list of jobs
 */
function job_teasers_list($page, $pageSize, $search)
{
    $jobs = array();
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'job')
        ->propertyCondition('status', NODE_PUBLISHED)
        ->range($page * $pageSize, $pageSize);

    if (isset($search) and strlen(trim($search)) > 0) {
//        $query->propertyCondition('title', '%' . db_like($search) . '%', 'like');
        $query->addTag('job_or_employer_title');
    }

    $result = $query->execute();
    if (isset($result['node'])) {
        $jobs_ids = array_keys($result['node']);
        foreach (entity_load('node', $jobs_ids) as $job) {
            array_push($jobs, get_job_resource($job));
        }
    }
    return $jobs;
}

/**
 * Implements hook_query_TAG_alter().
 */
function jir_mobile_resources_query_job_or_employer_title_alter(QueryAlterableInterface $query)
{
    $query->leftJoin('field_employer', 'emp', 'node.nid = emp.entity_id');
    $or = db_or()
        ->condition('node.title', '%' . db_like('Rwanda') . '%', 'like')
        ->condition('emp.title', '%' . db_like('Rwanda') . '%', 'like');
    $query->condition($or);
}

function get_job_resource($node)
{
    $employer_id = $node->field_employer[LANGUAGE_NONE][0]['target_id'];
    $employer = node_load($employer_id);

    $categories = array();
    foreach ($node->field_category[LANGUAGE_NONE] as $category) {
        array_push($categories, taxonomy_term_load($category['tid'])->name);
    }
    $job = array(
        'job_title' => $node->title,
        'job_id' => intval($node->nid),
        'path' => drupal_get_path_alias('node/' . $node->nid),
        'created_date' => format_date($node->created, 'short'),
        'contact_name' => $node->field_contact_name[LANGUAGE_NONE][0]['value'],
        'posting_type' => taxonomy_term_load($node->field_posting_type[LANGUAGE_NONE][0]['tid'])->name,
        'category' => $categories,
        'deadline' => date('Y-m-d H:i', strtotime($node->field_deadline_for_application[LANGUAGE_NONE][0]['value'])),
        'additional_email_cv' => isset($node->field_additional_email_where_to[LANGUAGE_NONE]) ? $node->field_additional_email_where_to[LANGUAGE_NONE][0]['email'] : '',
        'number_of_positions' => intval($node->field_number_of_positions[LANGUAGE_NONE][0]['value']),
        'contract_type' => taxonomy_term_load($node->field_contrat_type[LANGUAGE_NONE][0]['tid'])->name,
        'desired_experience' => taxonomy_term_load($node->field_desired_experience[LANGUAGE_NONE][0]['tid'])->name,
        'desired_education_level' => taxonomy_term_load($node->field_desired_education_level[LANGUAGE_NONE][0]['tid'])->name,
        'offer_type' => $node->field_offer_type[LANGUAGE_NONE][0]['value'],
        'city' => $node->field_job_city[LANGUAGE_NONE][0]['safe_value'],
        'country' => $node->field_job_country[LANGUAGE_NONE][0]['iso2'],
        'action_apply_default_url' => $node->field_apply_default[LANGUAGE_NONE][0]['url'],
        'application_form_type' => taxonomy_term_load($node->field_application_form_type[LANGUAGE_NONE][0]['tid'])->name,
        'body' => $node->body[LANGUAGE_NONE][0]['safe_value'],
        'is_featured_advert' => boolval($node->field_is_featured_advert[LANGUAGE_NONE][0]['value']),
        'is_super_featured' => boolval($node->field_job_super_featured[LANGUAGE_NONE][0]['value']),
        'employer' => array(
            'id' => intval($employer->nid),
            'name' => $employer->title,
            'logo_url' => file_create_url($employer->field_employer_logo[LANGUAGE_NONE][0]['uri']),
            'description' => $employer->field_summary[LANGUAGE_NONE][0]['value'],
        ),
    );
    return $job;
}