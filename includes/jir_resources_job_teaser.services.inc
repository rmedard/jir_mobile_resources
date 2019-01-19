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
                    )
                ),
                'get_advert_types_stats' => array(
                    'help' => 'Get advert types stats. How many jobs per type',
                    'file' => array(
                        'type' => 'inc',
                        'module' => 'jir_mobile_resources',
                        'name' => 'includes/jir_resources_job_teaser.services',
                    ),
                    'callback' => 'advert_types_statistics',
                    'access callback' => 'user_is_logged_in',
                    'args' => array()
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
            $job = get_job_full_resource($node);
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
    $smartQuery = new EntityFieldQueryExtraFields();
    $smartQuery->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'job')
        ->propertyCondition('status', NODE_PUBLISHED);

    $smartQuery->addExtraField('', 'title', null, 'node')
        ->addExtraField('field_posting_type', 'tid')
        ->addExtraField('field_deadline_for_application', 'value')
        ->addExtraField('field_contrat_type', 'tid')
        ->addExtraField('field_desired_experience', 'tid')
        ->addExtraField('field_offer_type', 'value')
        ->addExtraField('field_is_featured_advert', 'value')
        ->addExtraField('field_job_super_featured', 'value')
        ->addExtraField('field_employer', 'target_id');
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
    foreach ((object)$smartResult['node'] as $jobNode) {
        array_push($jobNodes, get_job_teaser_resource($jobNode));
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

function get_job_teaser_resource($node)
{
    $query = new EntityFieldQueryExtraFields();
    $query->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'employer')
        ->propertyCondition('nid', intval($node->extraFields->field_employer_));

    $query->addExtraField('', 'title', null, 'node')
        ->addExtraField('field_employer_logo', 'fid');

    $result = $query->execute();

    $employer = new stdClass();
    foreach ((object)$result['node'] as $emp) {
        $employer = $emp;
        break;
    }
    $employerLogo = isset($employer->extraFields->field_employer_logo_) ? file_load($employer->extraFields->field_employer_logo_) : null;
    $postingType = taxonomy_term_load($node->extraFields->field_posting_type_);

    return array(
        'job_title' => $node->extraFields->title,
        'job_id' => intval($node->nid),
        'path' => drupal_get_path_alias('node/' . $node->nid),
        'posting_type' => $postingType->name,
        'posting_type_weight' => intval($postingType->weight),
        'deadline' => date('Y-m-d H:i', strtotime($node->extraFields->field_deadline_for_application_)),
        'contract_type' => taxonomy_term_load($node->extraFields->field_contrat_type_)->name,
        'desired_experience' => taxonomy_term_load($node->extraFields->field_desired_experience_)->name,
        'offer_type' => $node->extraFields->field_offer_type_,
        'is_featured_advert' => boolval($node->extraFields->field_is_featured_advert_),
        'is_super_featured' => boolval($node->extraFields->field_job_super_featured_),
        'employer' => array(
            'id' => intval($employer->nid),
            'name' => $employer->extraFields->title,
            'logo_url' => isset($employerLogo) ? image_style_url('logo_scaler_new', $employerLogo->uri) : '',
        )
    );
}

function get_job_full_resource($node)
{
    $employerNode = node_load($node->field_employer[LANGUAGE_NONE][0]['target_id']);

    $jobCategories = array();
    if (isset($node->field_category[LANGUAGE_NONE]) and count($node->field_category[LANGUAGE_NONE]) > 0) {
        foreach ($node->field_category[LANGUAGE_NONE] as $category) {
            array_push($jobCategories, taxonomy_term_load($category['tid'])->name);
        }
    }

    $employerCategories = array();
    if (isset($employerNode->field_category) and count($employerNode->field_category[LANGUAGE_NONE]) > 0) {
        foreach ($employerNode->field_category[LANGUAGE_NONE] as $category) {
            array_push($employerCategories, taxonomy_term_load($category['tid'])->name);
        }
    }

    $postingTypeTaxonomy = taxonomy_term_load($node->field_posting_type[LANGUAGE_NONE][0]['tid']);

    statistics_node_view($node, 'full');

    $job = array(
        'job_title' => $node->title,
        'job_id' => intval($node->nid),
        'path' => drupal_get_path_alias('node/' . $node->nid),
        'created_date' => format_date($node->created, 'short'),
        'contact_name' => $node->field_contact_name[LANGUAGE_NONE][0]['value'],
        'posting_type' => $postingTypeTaxonomy->name,
        'posting_type_weight' => intval($postingTypeTaxonomy->weight),
        'category' => empty($jobCategories) ? 'Not specified' : $jobCategories,
        'deadline' => date('Y-m-d H:i', strtotime($node->field_deadline_for_application[LANGUAGE_NONE][0]['value'])),
        'additional_email_cv' => isset($node->field_additional_email_where_to[LANGUAGE_NONE]) ? $node->field_additional_email_where_to[LANGUAGE_NONE][0]['email'] : '',
        'number_of_positions' => intval($node->field_number_of_positions[LANGUAGE_NONE][0]['value']),
        'contract_type' => taxonomy_term_load($node->field_contrat_type[LANGUAGE_NONE][0]['tid'])->name,
        'desired_experience' => taxonomy_term_load($node->field_desired_experience[LANGUAGE_NONE][0]['tid'])->name,
        'desired_education_level' => taxonomy_term_load($node->field_desired_education_level[LANGUAGE_NONE][0]['tid'])->name,
        'offer_type' => $node->field_offer_type[LANGUAGE_NONE][0]['value'],
        'city' => isset($node->field_job_city) ? trim($node->field_job_city[LANGUAGE_NONE][0]['safe_value']) : '',
        'country' => isset($node->field_job_country) ? $node->field_job_country[LANGUAGE_NONE][0]['iso2'] : '',
        'action_apply_default_url' => $node->field_apply_default[LANGUAGE_NONE][0]['url'],
        'application_form_type' => taxonomy_term_load($node->field_application_form_type[LANGUAGE_NONE][0]['tid'])->name,
        'body' => $node->body[LANGUAGE_NONE][0]['safe_value'],
        'is_featured_advert' => boolval($node->field_is_featured_advert[LANGUAGE_NONE][0]['value']),
        'is_super_featured' => boolval($node->field_job_super_featured[LANGUAGE_NONE][0]['value']),
        'total_views_count' => intval(statistics_get($node->nid)['totalcount']),
        'employer' => array(
            'id' => intval($employerNode->nid),
            'sector' => $employerCategories,
            'description' => isset($employerNode->field_summary) ? $employerNode->field_summary[LANGUAGE_NONE][0]['safe_value'] : null,
            'website' => isset($employerNode->field_employer_website) ? $employerNode->field_employer_website[LANGUAGE_NONE][0]['url'] : null
        )
    );
    return $job;
}

function advert_types_statistics() {
    watchdog('Rest API', 'In stats...');
    $query1 = new EntityFieldQuery();
    $featured = $query1->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'job')
        ->propertyCondition('status', NODE_PUBLISHED)
        ->fieldCondition('field_posting_type', 'tid', [33, 34, 36,], 'IN')
        ->count()->execute();

    $query2 = new EntityFieldQuery();
    $jobs = $query2->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'job')
        ->propertyCondition('status', NODE_PUBLISHED)
        ->fieldCondition('field_offer_type', 'value', 'Job')
        ->count()->execute();

    $query3 = new EntityFieldQuery();
    $tenders = $query3->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'job')
        ->propertyCondition('status', NODE_PUBLISHED)
        ->fieldCondition('field_offer_type', 'value', 'Tender')
        ->count()->execute();

    $query4 = new EntityFieldQuery();
    $consultancy = $query4->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'job')
        ->propertyCondition('status', NODE_PUBLISHED)
        ->addTag('consultancy_or_freelance')
        ->count()->execute();

    $query5 = new EntityFieldQuery();
    $internship = $query5->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'job')
        ->propertyCondition('status', NODE_PUBLISHED)
        ->fieldCondition('field_offer_type', 'value', 'Internship')
        ->count()->execute();

    $query6 = new EntityFieldQuery();
    $emps = $query6->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'employer')
        ->propertyCondition('status', NODE_PUBLISHED)
        ->fieldCondition('field_employer_public_employer', 'value', 1)
        ->execute();
    $publics = 0;
    if (isset($emps['node'])) {
        $emps_ids = array_keys($emps['node']);
        $query7 = new EntityFieldQuery();
        $publics = $query7->entityCondition('entity_type', 'node')
            ->entityCondition('bundle', 'job')
            ->propertyCondition('status', NODE_PUBLISHED)
            ->fieldCondition('field_employer', 'target_id', $emps_ids, 'IN')
            ->count()->execute();
    }

    $query7 = new EntityFieldQuery();
    $others = $query7->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'job')
        ->propertyCondition('status', NODE_PUBLISHED)
        ->fieldCondition('field_offer_type', 'value', 'Other')
        ->count()->execute();

    watchdog('Rest API', $featured);

    return array(
        'FEATURED' => $featured,
        'JOBS' => $jobs,
        'TENDERS', $tenders,
        'INTERNSHIP' => $internship,
        'CONSULTANCY' => $consultancy,
        'OTHERS'=> $others,
        'PUBLIC' => $publics);
}

/**
 * Implements hook_query_TAG_alter().
 */
function jir_mobile_resources_query_consultancy_or_freelance_alter(QueryAlterableInterface $query)
{
    $query->leftjoin('field_data_field_offer_type', 'o', 'nid = o.entity_id');
    $query->leftjoin('field_data_field_contrat_type', 'c', 'nid = c.entity_id');
    $query->leftjoin('taxonomy_term_data', 'n', 'n.tid = c.field_contrat_type_tid');
    $or = db_or();
    $or->condition('o.field_offer_type_value', 'Consultancy')
        ->condition('n.name', 'Freelance');
    $query->condition($or);
}