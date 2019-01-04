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
                        array(
                            'name' => 'employer',
                            'type' => 'int',
                            'description' => 'ID of the employer to whom jobs belong',
                            'source' => array(
                                'param' => 'employer',
                            ),
                            'optional' => TRUE,
                            'default value' => 0,
                        ),
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
            $job = get_job_teasers_resource($node);
        }
    }
    return $job;
}

/**
 * @return array
 *  Return a list of jobs
 */
function job_teasers_list($pageSize, $search, $employerId)
{
//    $jobs = array();
//    $query = new EntityFieldQuery();
//    $query->entityCondition('entity_type', 'node')
//        ->entityCondition('bundle', 'job')
//        ->propertyCondition('status', NODE_PUBLISHED);
//
//    if (isset($search) and strlen(trim($search)) > 0) {
//        $query->addTag('job_or_employer_title')->addMetaData('search', $search);
//    }
//
//    if (isset($employerId) and intval($employerId) !== 0) {
//        $query->fieldCondition('field_employer', 'target_id', intval($employerId));
//    }
//    try {
//        $query->fieldOrderBy('field_job_super_featured', 'value', 'DESC')
//            ->fieldOrderBy('field_posting_type', 'tid', 'DESC');
//    } catch (EntityFieldQueryException $e) {
//        watchdog('error', $e);
//    }
//    $result = $query->pager($pageSize)->execute();
//
//    $counter = 0;
//    if (isset($result['node'])) {
//        $jobs_ids = array_keys($result['node']);
////        $resultFields = array();
////        field_attach_load('node', $result['node'], FIELD_LOAD_CURRENT, $resultFields);
//        foreach (entity_load('node', $jobs_ids) as $job) {
//            array_push($jobs, get_job_teasers_resource($job));
//            $counter++;
//        }
//    }


    $smartQuery = new EntityFieldQueryExtraFields();
    $smartQuery->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'job')
        ->propertyCondition('status', NODE_PUBLISHED);

    $smartQuery->addExtraField('', 'title', null, 'node')
//        ->addExtraField('', 'created')
//        ->addExtraField('field_contact_name', 'value')
        ->addExtraField('field_posting_type', 'tid')
        ->addExtraField('field_deadline_for_application', 'value')
        ->addExtraField('field_contrat_type', 'tid')
        ->addExtraField('field_desired_experience', 'tid')
        ->addExtraField('field_offer_type', 'value')
//        ->addExtraField('field_job_city', 'value')
//        ->addExtraField('field_job_country', 'iso2')
        ->addExtraField('field_is_featured_advert', 'value')
        ->addExtraField('field_job_super_featured', 'value')
        ->addExtraField('field_employer', 'target_id')
    ;
    if (isset($search) and strlen(trim($search)) > 0) {
        $smartQuery->addTag('job_or_employer_title')->addMetaData('search', $search);
    }

    if (isset($employerId) and intval($employerId) !== 0) {
        $smartQuery->fieldCondition('field_employer', 'target_id', intval($employerId));
    }
    try {
        $smartQuery->fieldOrderBy('field_job_super_featured', 'value', 'DESC')
            ->fieldOrderBy('field_posting_type', 'tid', 'DESC');
    } catch (EntityFieldQueryException $e) {
        watchdog('error', $e);
    }

    $smartResult = $smartQuery->pager($pageSize)->execute();

    $jobNodes = array();
    foreach ((object) $smartResult['node'] as $jobNode) {
        array_push($jobNodes, get_job_teasers_resource_new($jobNode));
    }
    return $jobNodes;
}

/**
 * Implements hook_query_TAG_alter().
 * March title of Job or Employer and sort everything by Posting Plan
 */
function jir_mobile_resources_query_job_or_employer_title_alter(QueryAlterableInterface &$query)
{
    $query->leftjoin('field_data_field_employer', 'e', 'nid = e.entity_id');
    $query->leftjoin('node', 'n', 'n.nid = e.field_employer_target_id');
    $search = $query->getMetaData('search');
    $or = db_or();
    $or->condition('node.title', '%' . db_like($search) . '%', 'like')
        ->condition('n.title', '%' . db_like($search) . '%', 'like');
    $query->condition($or);
}

function get_job_teasers_resource($node)
{
    $employer_id = $node->field_employer[LANGUAGE_NONE][0]['target_id'];
    $employer = node_load($employer_id);

//    $categories = array();
//    if (isset($node->field_category[LANGUAGE_NONE]) and count($node->field_category[LANGUAGE_NONE]) > 0) {
//        foreach ($node->field_category[LANGUAGE_NONE] as $category) {
//            array_push($categories, taxonomy_term_load($category['tid'])->name);
//        }
//    }

    $employerLogoUri = $employer->field_employer_logo[LANGUAGE_NONE][0]['uri'];

    $job = array(
        'job_title' => $node->title,
        'job_id' => intval($node->nid),
        'path' => drupal_get_path_alias('node/' . $node->nid),
//        'created_date' => format_date($node->created, 'short'),
//        'contact_name' => $node->field_contact_name[LANGUAGE_NONE][0]['value'],
        'posting_type' => taxonomy_term_load($node->field_posting_type[LANGUAGE_NONE][0]['tid'])->name,
        'posting_type_weight' => intval(taxonomy_term_load($node->field_posting_type[LANGUAGE_NONE][0]['tid'])->weight),
//        'category' => $categories,
        'deadline' => date('Y-m-d H:i', strtotime($node->field_deadline_for_application[LANGUAGE_NONE][0]['value'])),
//        'additional_email_cv' => isset($node->field_additional_email_where_to[LANGUAGE_NONE]) ? $node->field_additional_email_where_to[LANGUAGE_NONE][0]['email'] : '',
//        'number_of_positions' => intval($node->field_number_of_positions[LANGUAGE_NONE][0]['value']),
        'contract_type' => taxonomy_term_load($node->field_contrat_type[LANGUAGE_NONE][0]['tid'])->name,
        'desired_experience' => taxonomy_term_load($node->field_desired_experience[LANGUAGE_NONE][0]['tid'])->name,
//        'desired_education_level' => taxonomy_term_load($node->field_desired_education_level[LANGUAGE_NONE][0]['tid'])->name,
        'offer_type' => $node->field_offer_type[LANGUAGE_NONE][0]['value'],
        'city' => $node->field_job_city[LANGUAGE_NONE][0]['safe_value'],
        'country' => $node->field_job_country[LANGUAGE_NONE][0]['iso2'],
//        'action_apply_default_url' => $node->field_apply_default[LANGUAGE_NONE][0]['url'],
//        'application_form_type' => taxonomy_term_load($node->field_application_form_type[LANGUAGE_NONE][0]['tid'])->name,
//        'body' => $node->body[LANGUAGE_NONE][0]['safe_value'],
        'is_featured_advert' => boolval($node->field_is_featured_advert[LANGUAGE_NONE][0]['value']),
        'is_super_featured' => boolval($node->field_job_super_featured[LANGUAGE_NONE][0]['value']),
        'employer' => array(
            'id' => intval($employer->nid),
            'name' => $employer->title,
            'logo_url' => isset($employerLogoUri) ? image_style_url('logo_scaler_new', $employerLogoUri) : '',
//            'description' => isset($employer->field_summary[LANGUAGE_NONE]) ? $employer->field_summary[LANGUAGE_NONE][0]['value'] : '',
        ),
    );
    return $job;
}

function get_job_teasers_resource_new($node) {
    $query = new EntityFieldQueryExtraFields();
    $query->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'employer')
        ->propertyCondition('nid', intval($node->extraFields->field_employer_));
    $query->addExtraField('', 'title', null, 'node')
        ->addExtraField('field_employer_logo', 'fid');

    $result = $query->execute();
    $employer = new stdClass();
    foreach ((object) $result['node'] as $emp) {
        $employer = $emp;
        break;
    }
    $employerLogoUri = $employer->extraFields->field_employer_logo_;
    $postingType = taxonomy_term_load($node->extraFields->field_posting_type_);
    return array(
        'job_title' => $node->extraFields->title,
        'job_id' => intval($node->nid),
        'path' => drupal_get_path_alias('node/' . $node->nid),
//        'created_date' => format_date($node->created, 'short'),
//        'contact_name' => $node->field_contact_name[LANGUAGE_NONE][0]['value'],
        'posting_type' => $postingType->name,
        'posting_type_weight' => intval($postingType->weight),
//        'category' => $categories,
        'deadline' => date('Y-m-d H:i', strtotime($node->extraFields->field_deadline_for_application_)),
//        'additional_email_cv' => isset($node->field_additional_email_where_to[LANGUAGE_NONE]) ? $node->field_additional_email_where_to[LANGUAGE_NONE][0]['email'] : '',
//        'number_of_positions' => intval($node->field_number_of_positions[LANGUAGE_NONE][0]['value']),
        'contract_type' => taxonomy_term_load($node->extraFields->field_contrat_type_)->name,
        'desired_experience' => taxonomy_term_load($node->extraFields->field_desired_experience_)->name,
//        'desired_education_level' => taxonomy_term_load($node->field_desired_education_level[LANGUAGE_NONE][0]['tid'])->name,
        'offer_type' => $node->extraFields->field_offer_type_,
//        'city' => $node->field_job_city[LANGUAGE_NONE][0]['safe_value'],
//        'country' => $node->field_job_country[LANGUAGE_NONE][0]['iso2'],
//        'action_apply_default_url' => $node->field_apply_default[LANGUAGE_NONE][0]['url'],
//        'application_form_type' => taxonomy_term_load($node->field_application_form_type[LANGUAGE_NONE][0]['tid'])->name,
//        'body' => $node->body[LANGUAGE_NONE][0]['safe_value'],
        'is_featured_advert' => boolval($node->extraFields->field_is_featured_advert_),
        'is_super_featured' => boolval($node->extraFields->field_job_super_featured_),
        'employer' => array(
            'id' => intval($employer->nid),
            'name' => $employer->extraFields->title,
            'logo_url' => isset($employerLogoUri) ? image_style_url('logo_scaler_new', $employerLogoUri) : '',
//            'description' => isset($employer->field_summary[LANGUAGE_NONE]) ? $employer->field_summary[LANGUAGE_NONE][0]['value'] : '',
        )
    );
}
