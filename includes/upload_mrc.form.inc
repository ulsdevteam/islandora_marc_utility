<?php

/**
 * @file
 * Form for uploading and processing a MARC.
 */

/**
 * Form building function.
 */
function islandora_marc_utility_form($form, &$form_state) {
  if (!isset($form_state['stage'])) {
    $form_state['stage'] = 'step1';
  }
  $form = array();
  switch ($form_state['stage']) {
    case 'step1':
      return islandora_marc_utility_form_step1($form, $form_state);
     break;

    case 'step2':
      return islandora_marc_utility_form_step2($form, $form_state);
     break;

    case 'complete':
      return islandora_marc_utility_form_step2($form, $form_state, TRUE);
     break;
  }
  return $form;
}

function islandora_marc_utility_form_step1($form, &$form_state) {
  $values = isset($form_state['multistep_values']['rate_the_room']) ? $form_state['multistep_values']['rate_the_room'] : array();
  $form = islandora_marc_utility_common_upload_form_elements();

  $form['override_settings_wrapper'] = array(
    '#type' => 'fieldset',
    '#title' => t('Override settings'),
    'override' => array(
      '#type' => 'checkbox',
      '#title' => t('Override'),
      '#default_value' => 0,
      '#attributes' => array(
        'onchange' => 'set_override_elements(this)',
      ),
    ),
    'mrc_identifier_tag_xpath' => array(
      '#title' => t('XPath to MARCXML node'),
      '#type' => 'textfield',
      '#size' => 120,
      '#attributes' => array(
        'placeholder' => MRC_XPATH_PLACEHOLDER,
        'readonly' => 'readonly',
        'disabled' => 'disabled',
        'class' => array('disabled_text'),
      ),
      '#description' => t('This should be a legal XPath that returns the single ' .
        '"identifier" node that is to be matched with a possible object\'s MODS field value.'),
      '#default_value' => variable_get('islandora_marc_utility_mrc_identifier_tag_xpath', ''),
    ),
    'mods_solr_fieldname' => array(
      '#title' => t('MODS identifier Solr fieldname.'),
      '#type' => 'textfield',
      '#attributes' => array(
        'placeholder' => MODS_SOLR_FIELDNAME_PLACEHOLDER,
        'readonly' => 'readonly',
        'disabled' => 'disabled',
        'class' => array('disabled_text'),
      ),
      '#description' => t('Solr fieldname for the MODS identifier that corresponds ' .
        'to the mrc identifier (above).'),
      '#default_value' => variable_get('islandora_marc_utility_mods_solr_fieldname', ''),
    ),
  );

  // Display the upload form.
  $form['marc_upload'] = array(
    '#type' => 'file',
    '#title' => t('MARC (mrc or MARCXML) collection'),
    '#description' => t('A MARC collection containing the metadata for a number of objects.'),
    '#upload_location' => variable_get('islandora_marc_utility_temp_path', variable_get('file_temporary_path', '/tmp')),
    '#upload_validators' => array(
      'file_validate_extensions' => array(''),
    ),
  );
  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Next')
  );
  return $form;
}

function islandora_marc_utility_common_upload_form_elements() {
  $form = array();
  if (user_access(ISLANDORA_MARC_UTILITY_MANAGE)) {
    $form['admin'] = array(
        '#type' => 'item',
        '#markup' => l('Islandora MARC Utility - Settings',
              'admin/islandora/islandora_marc_utility',
              array('attributes' => array('target' => '_blank'))),
    );
  }
  return $form;
}

function islandora_marc_utility_form_step2($form, &$form_state, $is_completed = FALSE) {
  $form = islandora_marc_utility_common_upload_form_elements();
  // Display the Processing form.
  $form['name'] = array(
    '#type' => 'item',
    '#title' => t('Name'),
    '#description' => $_FILES['files']['name']['marc_upload'],
  );
  $form['type'] = array(
    '#type' => 'item',
    '#title' => t('Type'),
    '#description' => $_FILES['files']['type']['marc_upload'],
  );
  $form['tmp_name'] = array(
    '#type' => 'item',
    '#title' => t('tmp name'),
    '#description' => $_FILES['files']['tmp_name']['marc_upload'],
  );
  $form['size'] = array(
    '#type' => 'item',
    '#title' => t('Size (bytes)'),
    '#description' => number_format($_FILES['files']['size']['marc_upload']),
  );

  // call the function to perform the parsing of the collection file based on the file type.
  if ($_FILES['files']['type']['marc_upload'] == 'application/octet-stream') {
    $fn = 'islandora_marc_utility_mrc_split';
  }
  elseif ($_FILES['files']['type']['marc_upload'] == 'text/xml') {
    $fn = 'islandora_marc_utility_marcxml_split';
  }
  else {
    drupal_set_message(t('The uploaded file is not supported (mime type "<b><i>@uploaded_file_mimetype</i></b>").  The only supported ' .
        'file mime types are "<b>application/octet-stream</b>" and "<b>text/xml</b>".',
          array(
              '@uploaded_file_mimetype' => $_FILES['files']['type']['marc_upload'],
          )), 'error');
    $fn = '';
  }

  // If the user selected to override the configured xpath and MODS field, pass those
  // values in to whichever parsing function is needed for the uploaded file.
  $marc_identifier_xpath = variable_get('islandora_marc_utility_mrc_identifier_tag_xpath', MRC_XPATH_PLACEHOLDER);
  $mods_identifier_solr_fieldname = variable_get('islandora_marc_utility_mods_solr_fieldname', MODS_SOLR_FIELDNAME_PLACEHOLDER);
  if (isset($_POST['override']) && $_POST['override']) {
    if (isset($_POST['mrc_identifier_tag_xpath']) && $_POST['mrc_identifier_tag_xpath']) {
      $marc_identifier_xpath = $_POST['mrc_identifier_tag_xpath'];
    }
    if (isset($_POST['mods_solr_fieldname']) && $_POST['mods_solr_fieldname']) {
      $mods_identifier_solr_fieldname = $_POST['mods_solr_fieldname'];
    }
  }

  // This code will perform everything in the temp directory.
  $marc_and_mods_records = (($fn) ?
      $fn($_FILES['files']['tmp_name']['marc_upload'], $marc_identifier_xpath, $mods_identifier_solr_fieldname) :
      array());
  islandora_marc_utility_add_operation_links_and_title($marc_and_mods_records);
  $form['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#suffix' => ' ',
  );
  if (!$is_completed && $fn) {
    $header = array(
        'bib_id',
        'Fedora PID',
        'MARC',
        array('data' => 'MODS', 'title' => 'Transformed MARC into MODS v3.5 using MARC21slim2MODS3-5.xsl'),
        array('data' => 'MARC size', 'class' => array('imu_numeric_header')),
        array('data' => 'MODS size', 'class' => array('imu_numeric_header')),
    );
    $form['download_all'] = array(
      '#type' => 'submit',
      '#attributes' => array(
          'title' => 'Not implemented yet',
          'disabled' => 'disabled',
      ),
      '#value' => t('Download all'),
      '#suffix' => ' ',
    );
    $form['apply_all'] = array(
      '#type' => 'submit',
      '#value' => t('Apply all datastreams'),
      '#attributes' => array(
          'title' => 'Not implemented yet',
          'disabled' => 'disabled',
      ),
      '#suffix' => theme('table', array(
        'header' => $header,
        'rows' => $marc_and_mods_records)),
    );
  }

  return $form;
}

function islandora_marc_utility_add_operation_links_and_title(&$marc_and_mods_records) {
  foreach ($marc_and_mods_records as $index => $arr) {
    $marc_and_mods_records[$index]['MARC'] = islandora_marc_utility_get_operation_links('MARC', $arr);
    $marc_and_mods_records[$index]['MODS'] = islandora_marc_utility_get_operation_links('MODS', $arr);
    $marc_and_mods_records[$index]['MARC size'] = array(
        'data' => number_format($marc_and_mods_records[$index]['MARC size']),
          'class' => array('imu_numeric'),
        );
    $marc_and_mods_records[$index]['MODS size'] = array(
        'data' => number_format($marc_and_mods_records[$index]['MODS size']),
          'class' => array('imu_numeric'),
        );
  }
}

function islandora_marc_utility_get_operation_links($subject, $arr) {
  // Based on user permissions, make links to View, Download, Apply Datastream.
  $links_arr = array();

  $b64_filename_bib_id_subj = base64_encode($arr[$subject] . '|' .$arr['bib_id'] . '|' . $subject . '|' . $arr['Fedora_PID']);
  $file_contents = file_get_contents($arr[$subject]);
  $file_truncated_ellipsis = (strlen($file_contents) > 600) ? ' ... ' : '';

  // Download single file
  $links_arr[] = l('View', 'admin/islandora/tools/islandora_marc_utility/viewer/' . $b64_filename_bib_id_subj,
      array(
          'attributes' => array(
              'target' => '_blank'
      )));
  $links_arr[] = l('Download', 'admin/islandora/tools/islandora_marc_utility/download/' . $b64_filename_bib_id_subj,
      array(
          'attributes' => array(
              'target' => '_blank',
              'title' => 'Download ' . $subject . ' record'
      )));
  if (user_access(ISLANDORA_MARC_UTILITY_CAN_APPLY_DATASTREAM) && isset($arr['Fedora_PID']) && $arr['Fedora_PID']) {
    $links_arr[] = l('Apply', 'admin/islandora/tools/islandora_marc_utility/apply_datastream/' . $b64_filename_bib_id_subj,
        array(
            'attributes' => array(
                'target' => '_blank',
                'title' => 'Apply ' . $subject . ' record as object datastream'
        )));
  }

  return array(
      'data' => implode(" ", $links_arr),
      'title' => substr($file_contents, 0, 600) . $file_truncated_ellipsis);
}

/**
 * Form submit handler.
 *
 * May kick off a batch process.
 */
function islandora_marc_utility_form_submit($form, &$form_state) {
  switch ($form_state['stage']) {
    case 'step2':
      $form_state['multistep_values'][$form_state['stage']] = $form_state['values'];
      if ($form_state['triggering_element']['#value'] != 'Cancel') {
        $form_state['complete'] = TRUE;
      }
     break;

    default:
      $form_state['multistep_values'][$form_state['stage']] = $form_state['values'];
     break;
  }

  if ($form_state['triggering_element']['#value'] == 'Cancel') {
    $form_state['new_stage'] = islandora_marc_utility_move_to_previous_stage($form, $form_state);
  }
  else {
    $form_state['new_stage'] = islandora_marc_utility_move_to_next_stage($form, $form_state);
  }

  if (isset($form_state['multistep_values']['form_build_id'])) {
    $form_state['values']['form_build_id'] = $form_state['multistep_values']['form_build_id'];
  }
  $form_state['multistep_values']['form_build_id'] = $form_state['values']['form_build_id'];

  $form_state['stage'] = $form_state['new_stage'];
  $form_state['rebuild'] = TRUE;
}

function islandora_marc_utility_move_to_next_stage($form, &$form_state) {
  switch ($form_state['stage']) {
    case 'step1':
      return 'step2';
     break;

    case 'step2':
      return 'complete';
     break;
  }
}

function islandora_marc_utility_move_to_previous_stage($form, &$form_state) {
  switch ($form_state['stage']) {
    case 'step2':
      return 'step1';
     break;
  }
}

/**
 * Helper function to run an XSL transform on the provided $info['input'] file.
 *
 * @param array $info
 *   This array contains the xsl value and the input (xml) filenames.
 * @return string
 *   The content of the transformed result.
 */
function islandora_marc_utility_runXslTransform($info) {
  $xsl = new DOMDocument('1.0', 'utf-8');
  $xsl->load($info['xsl']);
  $input = new DOMDocument('1.0', 'utf-8');
  $input->loadXML($info['input']);

  $processor = new XSLTProcessor();
  $processor->importStylesheet($xsl);

  if (isset($info['php_functions'])) {
    // Register php functions limited to those provided in the
    // $info['php_functions'] value.
    $processor->registerPHPFunctions($info['php_functions']);
  } elseif (isset($info['php_functions_all']) && $info['php_functions_all']) {
    // No restriction on the functions that are registered in the xslt processor.
    $processor->registerPHPFunctions();
  }

  return $processor->transformToXML($input);
}

function islandora_marc_utility_mrc_split($filepath, $marc_identifier_xpath, $mods_identifier_solr_fieldname) {
  module_load_include('inc', 'islandora_marc_utility', 'includes/solr_query');

  $return_files = array();

  require_once 'File/MARC.php';

  // Retrieve a set of MARC records from a file
  $journals = new File_MARC($filepath);
  $i = 0;
  // Iterate through the retrieved records
  while ($record = $journals->next()) {
    $results = islandora_marc_utility_get_marc_mods_bib_id_from_record($record, $i);
    $return_files[$i]['bib_id'] = $results['bib_id'];
    $marc_contents = file_get_contents($results['MARC']);
    $return_files[$i]['Fedora_PID'] = islandora_marc_utility_find_object_pid_from_marcxml_file($marc_contents, $marc_identifier_xpath, $mods_identifier_solr_fieldname);
    $return_files[$i]['MARC'] = $results['MARC'];
    $return_files[$i]['MODS'] = $results['MODS'];
    $return_files[$i]['MARC size'] = filesize($results['MARC']);
    $return_files[$i]['MODS size'] = filesize($results['MODS']);
    $i++;
  }
  return $return_files;
}

function islandora_marc_utility_get_marc_mods_bib_id_from_record($recordobj_or_xml, $index = 0) {
  $marc2mods_xsl = drupal_realpath(drupal_get_path('module', 'islandora_marc_utility')) .
        '/transforms/MARC21slim2MODS3-5.xsl';
  $modsxmlRecordExtract_xsl = drupal_realpath(drupal_get_path('module', 'islandora_marc_utility')) .
        '/transforms/modsxmlRecordExtract.xsl';
  $marcxmlRecordExtract_xsl = drupal_realpath(drupal_get_path('module', 'islandora_marc_utility')) .
        '/transforms/marcxmlRecordExtract.xsl';
  $voyagerIDgrabber_xsl = drupal_realpath(drupal_get_path('module', 'islandora_marc_utility')) .
        '/transforms/voyagerIDgrabber.xsl';

  $return_array = array();
  $temp_path = drupal_realpath(variable_get('islandora_marc_utility_temp_path', variable_get('file_temporary_path', '/tmp')));
  $filename = $temp_path . '/marc_record_' . $index . '.mrc.xml';
  $tmp = file_put_contents($filename, $recordobj_or_xml);
  $marcxml_file = fopen($filename, "wb");

  if (is_object($recordobj_or_xml)) {
    $marcxml_collection_record = $recordobj_or_xml->toXML();
    fwrite($marcxml_file, $marcxml_collection_record);
    fclose($marcxml_file);

    $marc_record = ($marcxml_collection_record) ? islandora_marc_utility_runXslTransform(
              array(
                'xsl' => $marcxmlRecordExtract_xsl,
                'input' => $marcxml_collection_record,
              )
            ) : '';
  }
  else {
    $marc_record = $recordobj_or_xml;
    $marcxml_collection_record = $recordobj_or_xml;
  }

  $bib_id = ($marcxml_collection_record) ? islandora_marc_utility_runXslTransform(
            array(
              'xsl' => $voyagerIDgrabber_xsl,
              'input' => $marcxml_collection_record,
            )
          ) : '';
  // Delete the initial journal record collection file and save this using the
  // $bib_id value
  @unlink($filename);

  $filename = $temp_path . '/' . $bib_id . '.marcxml.xml';

  $bytes_written = file_put_contents($filename, $marc_record);
  $return_array['bib_id'] = $bib_id;
  $return_array['MARC'] = $filename;

  $modsCollection = ($marcxml_collection_record) ? islandora_marc_utility_runXslTransform(
            array(
              'xsl' => $marc2mods_xsl,
              'input' => $marcxml_collection_record,
            )
          ) : '';
  $MODS = ($modsCollection) ? islandora_marc_utility_runXslTransform(
            array(
              'xsl' => $modsxmlRecordExtract_xsl,
              'input' => $modsCollection,
            )
          ) : '';

  $filename = $temp_path . '/' . $bib_id . '.mods.xml';
  $bytes_written = file_put_contents($filename, $MODS);
  $return_array['MODS'] = $filename;

  return $return_array;
}

// Takes a MARCXML file and returns same as the MARC mrc code above.
function islandora_marc_utility_marcxml_split($filepath, $marc_identifier_xpath, $mods_identifier_solr_fieldname) {
  module_load_include('inc', 'islandora_marc_utility', 'includes/solr_query');

  $return_files = array();
  $marcxml = file_get_contents($filepath);

  // Use PHP's DOM Document to pull out each of the MARCXML records.
  $doc  = new DOMDocument('1.0', 'utf-8'); // DomDocument();
  $doc->loadXML($marcxml);
  $xpath = new DOMXPath($doc);
  $xpath->registerNamespace("marc", "http://www.loc.gov/MARC21/slim");

  $i = 0;
  foreach ($xpath->evaluate('//marc:record') as $node) {
    $str = $doc->saveHTML($node);
    $str = str_replace("<marc:record>", '<marc:record xmlns:marc="http://www.loc.gov/MARC21/slim">', $str);

    $results = islandora_marc_utility_get_marc_mods_bib_id_from_record($str, $i);
    $return_files[$i]['bib_id'] = $results['bib_id'];
    $marc_contents = file_get_contents($results['MARC']);
    $return_files[$i]['Fedora_PID'] = islandora_marc_utility_find_object_pid_from_marcxml_file($marc_contents, $marc_identifier_xpath, $mods_identifier_solr_fieldname);
    $return_files[$i]['MARC'] = $results['MARC'];
    $return_files[$i]['MODS'] = $results['MODS'];
    $return_files[$i]['MARC size'] = filesize($results['MARC']);
    $return_files[$i]['MODS size'] = filesize($results['MODS']);
    $i++;
  }
  return $return_files;
}