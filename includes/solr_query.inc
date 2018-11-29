<?php

/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

function islandora_marc_utility_query_solr_for_mrc_value($mrc_value) {
  // Based on the configuration, run an xpath query on the file to get the
  // identifier value from the
}

function islandora_marc_utility_find_object_pid_from_marcxml_file($marcxml_file, $marc_identifier_xpath, $mods_identifier_solr_fieldname) {
  // 1. parse the marcxml file to get the configured MARC identifier value.
  //    Uses variable_get('islandora_marc_utility_mrc_identifier_tag_xpath',
  //    '/marc:record/marc:datafield[tag="35"]/marc:subfield[code="a"]').
  $marc_identifier_value = islandora_marc_utility_get_marc_identifer_value(
      $marcxml_file, $marc_identifier_xpath, 'marc', 'http://www.loc.gov/MARC21/slim');

  $found_object_PID = NULL;
  if ($marc_identifier_value) {
    // 2. run a SOLR query to find the object that matches the configured field
    //    with that value.  Uses variable_get('islandora_marc_utility_mods_solr_fieldname',
    //    'mods_identifier_oclc_s'),
    $filters = array($mods_identifier_solr_fieldname . ':' . $marc_identifier_value);
    $found_object_PID = islandora_marc_utility_solr_query_for_object('PID', $filters, 1);
  }
  return $found_object_PID;
}

function islandora_marc_utility_get_marc_identifer_value($xml_content, $xpath, $namespace = '', $namespace_uri = '') {
  $xml = new SimpleXMLElement($xml_content);
  if ($namespace && $namespace_uri) {
    $xml->registerXPathNamespace($namespace, $namespace_uri);
  }
  $match = '';
  $match_results = $xml->xpath($xpath);
  if (is_array($match_results) && count($match_results) > 0) {
    $matches = $match_results[0];
    $s = (string)$matches;
    // Sanitize that value to remove prefix, special characters, hoopla, etc.
    $match = str_replace(array(
        '(OCoLC)', '(WlCaITV)', ''
        ), "", $s);
  }
  return $match;
}

function islandora_marc_utility_solr_query_for_object($field, $filters, $rows) {
  $value = NULL;
  $query_processor = new IslandoraSolrQueryProcessor();

  $query_processor->solrQuery = implode(" AND ", $filters);
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = $rows;
  $query_processor->solrParams = array(
    'fl' => $field,
    'fq' => '',
  );
  $query_processor->executeQuery(FALSE);
  if ($query_processor->islandoraSolrResult['response']['numFound'] > 0) {
    $solr_results_doc = $query_processor->islandoraSolrResult['response']['objects']['0']['solr_doc'];
    $value = (isset($solr_results_doc[$field]) && is_array($solr_results_doc[$field])) ? (($single_result) ? $solr_results_doc[$field][0] : implode("<br>", $solr_results_doc[$field])) :
      (isset($solr_results_doc[$field]) ? $solr_results_doc[$field] : '');
  }
  return $value;
}