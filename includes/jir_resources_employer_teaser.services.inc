<?php
/**
 * @file
 * Actual Resource
 */

function employer_teaser_resource()
{

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

function employer_teaser_resource_retrieve($nid)
{
    $node = node_load($nid);
    $employer = array();
    if ($node and is_object($node)) {
        if ($node->type == 'employer') {
            $employer = get_employer_full_resource($node);
        }
    }
    return $employer;
}

function employer_teasers_list($pageSize, $search)
{
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'employer')
        ->propertyCondition('status', NODE_PUBLISHED)
        ->addTag('employer_has_jobs');
    $result = $query->pager($pageSize)->execute();

    $employers = array();
    if (isset($result['node'])) {
        $employer_ids = array_keys($result['node']);
        $employerNodes = entity_load('node', $employer_ids);
        foreach ($employerNodes as $node) {
            array_push($employers, map_employer_teaser($node));
        }
    }
    return $employers;
}

function map_employer_teaser($node)
{
    $employerLogoUri = $node->field_employer_logo[LANGUAGE_NONE][0]['uri'];
    return array(
        'id' => $node->nid,
        'name' => $node->title,
        'logo_url' => isset($employerLogoUri) ? image_style_url('logo_scaler_new', $employerLogoUri) : ''
    );
}

/**
 * Implements hook_query_TAG_alter().
 */
function jir_mobile_resources_query_employer_has_jobs_alter(QueryAlterableInterface $query)
{
    if ($query instanceof SelectQueryInterface) {

//        $selectQuery = db_select('field_data_field_employer', 'e');
//        $selectQuery->innerJoin('node', 'n', 'n.nid = e.entity_id AND n.type = \'job\' AND n.status = 1');
//        $selectQuery->where('nid = e.field_employer_target_id')->groupBy('e.field_employer_target_id');
//        $query->exists($selectQuery);

//        SELECT * FROM node emp
//        WHERE EXISTS( SELECT job.nid FROM node job INNER JOIN field_data_field_employer e ON e.entity_id = job.nid
//              WHERE e.field_employer_target_id = emp.nid AND job.type = 'job' AND job.status = 1)
//AND emp.type = 'employer' AND emp.status = 1 ORDER BY emp.title ASC

//        $selectQuery = db_select('node', 'j')
//            ->condition('j.type', 'job')
//            ->condition('j.status', NODE_PUBLISHED)
//            ->condition('e.field_employer_target_id', 'nid')
//            ->fields('j', array('nid'));

        $selectQuery = db_select('node', 'j')->fields('j', array('nid'))
            ->condition('j.type', 'job', '=')
            ->condition('j.status', NODE_PUBLISHED, '=');
        $selectQuery->where('e.field_employer_target_id = nid')
            ->innerJoin('field_data_field_employer', 'e', 'j.nid = e.entity_id');
//            $selectQuery->where('e.field_employer_target_id = nid');
//        $selectQuery->where('e.field_employer_target_id = nid');
//        $selectQuery->condition('e.field_employer_target_id = nid');

        $query->exists($selectQuery);

//        $query->innerJoin('field_data_field_employer', 'e', 'nid = e.field_employer_target_id');
//        $query->innerJoin('node', 'n', 'n.nid = e.entity_id AND n.type = \'job\' AND n.status = 1');
//        $query->groupBy('e.field_employer_target_id');
    }
}

function get_employer_full_resource($node)
{
    $employerCategories = array();
    if (isset($node->field_category[LANGUAGE_NONE]) and count($node->field_category[LANGUAGE_NONE]) > 0) {
        foreach ($node->field_category[LANGUAGE_NONE] as $category) {
            array_push($employerCategories, taxonomy_term_load($category['tid'])->name);
        }
    }
    return array(
        'id' => intval($node->nid),
        'sector' => $employerCategories,
        'description' => isset($node->field_summary) ? $node->field_summary[LANGUAGE_NONE][0]['safe_value'] : null,
        'website' => isset($node->field_employer_website) ? $node->field_employer_website[LANGUAGE_NONE][0]['url'] : null
    );
}