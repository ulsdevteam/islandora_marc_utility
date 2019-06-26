<?php

/**
 * @file
 * Form for uploading and processing a MARC.
 */

/**
 * Form building function.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @return array
 *   The Drupal form definition.
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

/**
 * Prompt the user for the MARC file upload and option to override the settings.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @return array
 *   The Drupal form definition.
 */
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
    'namespace' => array(
      '#title' => t('Fedora namespace'),
      '#type' => 'textfield',
      '#attributes' => array(
        'placeholder' => 'islandora',
        'readonly' => 'readonly',
        'disabled' => 'disabled',
        'class' => array('disabled_text'),
        ),
      '#description' => t('Namespace for objects in the datastore.  For an object ' .
        '"<b>islandora</b>:123", the namespace is "islandora".  ') .
        (module_exists('islandora_datastreams_io') ?  t('"Islandora Datastreams ' .
            'Input/Output Utility" is installed and')
          : t('The module "') . l('Datastreams Input/Output Utility',
              'https://github.com/ulsdevteam/islandora_datastreams_io') . '"') .
        t(' is capable of importing the ZIP files that are created by this utility\'s ' .
            '"Download all" function.')
        ,
      '#default_value' => variable_get('islandora_marc_utility_namespace'),
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

/**
 * Helper function to return the array elements that would be common to all
 * steps of processing the MARC collection.
 *
 * @return array
 *   Elements for a Drupal form definition.
 *
 */
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

/**
 * The form to display the second / final step of parsing the MARC collection file.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param type $is_completed
 *   Whether or not the parsing of the underlying MARC has been completed or if
 * it still needs to be parsed.
 * @return array
 *   The Drupal form definition.
 */
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
    '#title' => t('Temporary file name'),
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
  $namespace = variable_get('islandora_marc_utility_namespace', 'islandora');
  if (isset($_POST['override']) && $_POST['override']) {
    if (isset($_POST['mrc_identifier_tag_xpath']) && $_POST['mrc_identifier_tag_xpath']) {
      $marc_identifier_xpath = $_POST['mrc_identifier_tag_xpath'];
    }
    if (isset($_POST['mods_solr_fieldname']) && $_POST['mods_solr_fieldname']) {
      $mods_identifier_solr_fieldname = $_POST['mods_solr_fieldname'];
    }
    if (isset($_POST['namespace']) && $_POST['namespace']) {
      $namespace = $_POST['namespace'];
    }
  }

  // This code will perform everything in the temp directory.
  $marc_and_mods_records = (($fn) ?
      $fn($_FILES['files']['tmp_name']['marc_upload'], $marc_identifier_xpath, $mods_identifier_solr_fieldname, $namespace) :
      array());
  islandora_marc_utility_add_operation_links_and_title($marc_and_mods_records);
  $form['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#suffix' => ' ',
  );
  $objects_found_by_match = 0;
  if (!$is_completed && $fn) {
    $header = array(
        'bib_id',
        'Fedora PID',
        'MARC',
        array('data' => 'MODS', 'title' => 'Transformed MARC into MODS v3.5 using MARC21slim2MODS3-5.xsl'),
        array('data' => 'MARC size', 'class' => array('imu_numeric_header')),
        array('data' => 'MODS size', 'class' => array('imu_numeric_header')),
    );
    $temp_path = drupal_realpath(variable_get('islandora_marc_utility_temp_path', variable_get('file_temporary_path', '/tmp')));
    $applyall_files = $download_files = array();
    foreach ($marc_and_mods_records as $idx => $marc_and_mods_arr) {
      if (array_key_exists('MARC', $marc_and_mods_arr) && array_key_exists('filepath', $marc_and_mods_arr['MARC'])) {
        $download_files[] = $marc_and_mods_arr['MARC']['filepath'] . '
';
        if ($marc_and_mods_arr['Fedora_PID']) {
          $applyall_files[] = $marc_and_mods_arr['Raw_Fedora_PID'] . '=' . $marc_and_mods_arr['MARC']['filepath'] . '
';
        }
      }
      if (array_key_exists('MODS', $marc_and_mods_arr) && array_key_exists('filepath', $marc_and_mods_arr['MODS'])) {
        $download_files[] = $marc_and_mods_arr['MODS']['filepath'] . '
';
        if ($marc_and_mods_arr['Fedora_PID']) {
          $applyall_files[] = $marc_and_mods_arr['Raw_Fedora_PID'] . '=' . $marc_and_mods_arr['MODS']['filepath'] . '
';
        }
      }
      if ($marc_and_mods_arr['Fedora_PID']) {
        $objects_found_by_match++;
      }
      unset($marc_and_mods_records[$idx]['Raw_Fedora_PID']);
    }

    $temp_records_manifest_filename = tempnam($temp_path, 'marc_utility_downloadall_manifest_');
    file_put_contents($temp_records_manifest_filename, $download_files);
    $temp_applyall_manifest_filename = tempnam($temp_path, 'marc_utility_applyall_manifest_');
    file_put_contents($temp_applyall_manifest_filename, $applyall_files);
    $form['download_all_link'] = array(
      '#type' => 'item',
      '#markup' => l(t('Download All'), 'admin/islandora/tools/islandora_marc_utility/downloadall/' . base64_encode(urlencode($temp_records_manifest_filename)), array('attributes' => array(
          'class' => array('m_link_button'),
          'target' => '_blank'))),
      '#suffix' => ' ',
    );
    $table_markup = theme('table', array(
        'header' => $header,
        'rows' => $marc_and_mods_records));
    if (user_access(ISLANDORA_MARC_UTILITY_CAN_APPLY_DATASTREAM)) {
      $form['apply_all'] = array(
        '#type' => 'item',
        '#markup' => (($objects_found_by_match > 0) ?
              l(t('Apply all datastreams'),
                  'admin/islandora/tools/islandora_marc_utility/applyall/' . base64_encode(urlencode($temp_applyall_manifest_filename)), array('attributes' => array(
              'class' => array('m_link_button'),
            'target' => '_blank'))) 
            : '<span title="There were no matching objects found for the records below" class="m_link_button b_disabled_text">Apply all datastreams</span>'),
        '#suffix' => $table_markup,
      );
    }
    else {
      $form['download_all_link']['#suffix'] = $table_markup;
    }
  }

  return $form;
}

/**
 * To put together a return array that is structured differently for a set of
 * table rows. This will change that variable by reference.
 *
 * @param array $marc_and_mods_records
 *   The raw data about the MARC xml and MODS xml that were generated by parsing
 * the MARC collection.
 */
function islandora_marc_utility_add_operation_links_and_title(&$marc_and_mods_records) {
  foreach ($marc_and_mods_records as $index => $arr) {
    $marc_and_mods_records[$index]['MARC'] = islandora_marc_utility_get_operation_links('MARC', $arr);
    $marc_and_mods_records[$index]['MODS'] = islandora_marc_utility_get_operation_links('MODS', $arr);
    $marc_and_mods_records[$index]['Raw_Fedora_PID'] = $marc_and_mods_records[$index]['Fedora_PID'];
    $marc_and_mods_records[$index]['Fedora_PID'] =  ($marc_and_mods_records[$index]['Fedora_PID']) ? l($marc_and_mods_records[$index]['Fedora_PID'],
            'islandora/object/' . $marc_and_mods_records[$index]['Fedora_PID'] . '/manage',
      array(
          'attributes' => array(
              'target' => '_blank'
      ))) : '';
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

/**
 * Helper function to make a set of links related to the MARC or MODS files
 * that came out of parsing the MARC collection.  These files use base64 to
 * encode the filename -- and it passes this to be handled by the route:
 *   admin/islandora/tools/islandora_marc_utility/viewer/{b64_value}
 *   admin/islandora/tools/islandora_marc_utility/download/{b64_value}
 *   admin/islandora/tools/islandora_marc_utility/apply_datastream/{b64_value}
 *
 * @param string $subject
 *   'MARC' or 'MODS'
 * @param type $arr
 * @return type
 */
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
      'filepath' => $arr[$subject],
      'data' => implode(" | ", $links_arr),
      'title' => substr($file_contents, 0, 600) . $file_truncated_ellipsis);
}

/**
 * Validates the form input.
 *
 * Checks the namespace value is legal.
 *
 * @param array $elements
 *   Drupal elements representation as an array.
 * @param array $form_state
 *   Drupal form_state representation as passed to the validate function.
 * @param string $form_id
 *   The form_id that is being validated -- optional.
 */
function islandora_marc_utility_form_validate(&$elements, &$form_state, $form_id = NULL) {
  // Check that the input for namespace satisfies this:
  $namespace = $form_state['values']['namespace'];
  if ($namespace <> '' && !preg_match("/^([A-Za-z0-9]{2,50}|-|\.)$/iD", $namespace)) {
    form_set_error('namespace', t('<b>Namespace value must only include alphanumeric characters.</b>',
        array('@namespace' => $namespace)));
  }
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
      if ($form_state['triggering_element']['#value'] == t('Download All')) {
        $form_state['complete'] = TRUE;
        $files = explode("\t", $form_state['values']['download_files']);
        drupal_set_message(t('"Download All" should be done.'), 'info');
        return;
      } elseif ($form_state['triggering_element']['#value'] != 'Cancel') {
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
  $form_state['rebuild'] = ($form_state['triggering_element']['#value'] == t('Download All')) ? FALSE : TRUE;
}

/**
 * Helper for setting the next stage of processing on this multi-step form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @return string
 *   The name of the stage to be used in determining which form to display.
 */
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

/**
 * Helper for setting the previous stage of processing on this multi-step form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @return string
 *   The name of the stage to be used in determining which form to display.
 */
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

/**
 * Code to parse a MARC MRC format file.
 *
 * All parameters are optional aside from the $filepath.  When this is called
 * from another module, the configuration values for most of the functionality
 * behind linking an object and applying the datastream to the right object is
 * not handled here.  That module must know what to do with the MARC and MODS
 * xml files.
 *
 * @param string $filepath
 *   The path to the uploaded MARC MRC collection file.
 * @param string $marc_identifier_xpath
 *   The value to determine the identifier from a MARC xml that comes from parsing
 * the MARC collection.
 * @param string $mods_identifier_solr_fieldname
 *   The value to search the existing Islandora repository for an object that
 * matches the MARC identifier value.
 * @param string $namespace
 *   The fedora namespace for the intended objects - ONLY for the purposes of
 * filenames that get created for export.  See "Islandora Datastreams Input/Output
 * Utility" https://github.com/ulsdevteam/islandora_datastreams_io for more info
 * on importing ZIP files.
 * @return array
 *   An associative array that contains info about the MARC and MODS files that
 * were able to be parsed out of the MARC mrc collection.
 */
function islandora_marc_utility_mrc_split($filepath, $marc_identifier_xpath = '', $mods_identifier_solr_fieldname = '', $namespace = '') {
  module_load_include('inc', 'islandora_marc_utility', 'includes/solr_query');

  $marc_identifier_xpath = (!$marc_identifier_xpath) ?
      variable_get('islandora_marc_utility_mrc_identifier_tag_xpath', MRC_XPATH_PLACEHOLDER) : $marc_identifier_xpath;
  $mods_identifier_solr_fieldname = (!$mods_identifier_solr_fieldname) ?
      variable_get('islandora_marc_utility_mods_solr_fieldname', MODS_SOLR_FIELDNAME_PLACEHOLDER) : $mods_identifier_solr_fieldname;
  $namespace = (!$namespace) ?
      variable_get('islandora_marc_utility_namespace', 'islandora') : $namespace;
  $return_files = array();

  if (!islandora_marc_utility_require_once_catch_exception('File/MARC.php')) {
    return array();
  }

  // Retrieve a set of MARC records from a file
  $journals = new File_MARC($filepath);
  $i = 0;
  // Iterate through the retrieved records
  while ($record = $journals->next()) {
    $results = islandora_marc_utility_get_marc_mods_bib_id_from_record($record, $i, $namespace);
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

/**
 * Helper function to get the identifier value from any MARC xml.
 *
 * @param mixed $recordobj_or_xml
 *   Object or string representation of the XML.
 * @param integer $index
 *   The index of the extracted marcxml from the File/MARC.php parsing.
 * @return string
 *   The identifier value.
 */
function islandora_marc_utility_get_marc_mods_bib_id_from_record($recordobj_or_xml, $index = 0, $namespace) {
  $marc2mods_xsl = drupal_realpath(drupal_get_path('module', 'islandora_marc_utility')) .
        '/transforms/MARC21slim2MODS3-5.xsl';
  $modsxmlRecordExtract_xsl = drupal_realpath(drupal_get_path('module', 'islandora_marc_utility')) .
        '/transforms/modsxmlRecordExtract.xsl';
  $marcxmlRecordExtract_xsl = drupal_realpath(drupal_get_path('module', 'islandora_marc_utility')) .
        '/transforms/marcxmlRecordExtract.xsl';
  $bibIDgrabber_xsl = drupal_realpath(drupal_get_path('module', 'islandora_marc_utility')) .
        '/transforms/bibIDgrabber.xsl';

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
              'xsl' => $bibIDgrabber_xsl,
              'input' => $marcxml_collection_record,
            )
          ) : '';
  // Delete the initial journal record collection file and save this using the
  // $bib_id value
  @unlink($filename);

  $filename = $temp_path . '/' . $namespace. '_'.  $bib_id . '_MARC.xml';

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

  $filename = $temp_path . '/' . $namespace. '_'.  $bib_id . '_MODS.xml';
  $bytes_written = file_put_contents($filename, $MODS);
  $return_array['MODS'] = $filename;

  return $return_array;
}

/**
 * Code to parse a MARCXML format file.
 *
 * All parameters are optional aside from the $filepath.  When this is called
 * from another module, the configuration values for most of the functionality
 * behind linking an object and applying the datastream to the right object is
 * not handled here.  That module must know what to do with the MARC and MODS
 * xml files.
 *
 * @param string $filepath
 *   The path to the uploaded MARC MRC collection file.
 * @param string $marc_identifier_xpath
 *   The value to determine the identifier from a MARC xml that comes from parsing
 * the MARC collection.
 * @param string $mods_identifier_solr_fieldname
 *   The value to search the existing Islandora repository for an object that
 * matches the MARC identifier value.
 * @param string $namespace
 *   The fedora namespace for the intended objects - ONLY for the purposes of
 * filenames that get created for export.  See "Islandora Datastreams Input/Output
 * Utility" https://github.com/ulsdevteam/islandora_datastreams_io for more info
 * on importing ZIP files.
 * @return array
 *   An associative array that contains info about the MARC and MODS files that
 * were able to be parsed out of the MARC mrc collection.
 */
function islandora_marc_utility_marcxml_split($filepath, $marc_identifier_xpath = '', $mods_identifier_solr_fieldname = '', $namespace = '') {
  module_load_include('inc', 'islandora_marc_utility', 'includes/solr_query');

  $marc_identifier_xpath = (!$marc_identifier_xpath) ?
      variable_get('islandora_marc_utility_mrc_identifier_tag_xpath', MRC_XPATH_PLACEHOLDER) : $marc_identifier_xpath;
  $mods_identifier_solr_fieldname = (!$mods_identifier_solr_fieldname) ?
      variable_get('islandora_marc_utility_mods_solr_fieldname', MODS_SOLR_FIELDNAME_PLACEHOLDER) : $mods_identifier_solr_fieldname;
  $namespace = (!$namespace) ?
      variable_get('islandora_marc_utility_namespace', 'islandora') : $namespace;

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

    $results = islandora_marc_utility_get_marc_mods_bib_id_from_record($str, $i, $namespace);
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

/**
 * Helper function to handle the require_once call that could lead to a white
 * screen.
 *
 * @param string $filepath
 *   The include file that is to be passed to the "require_once" call.
 * @throws Exception
 */
function islandora_marc_utility_require_once_catch_exception($filepath) {

  try {
    if (! @include_once( $filepath )) // @ - to suppress warnings,
    // Could use error_reporting function for the same purpose.
      throw new Exception (t('"@filepath" does not exist',array(
          '@filepath' => $filepath,
      )));
  }
  catch(Exception $e) {
    $message = '<b>Required file could not be loaded for the "Islandora MARC Utility" module.</b>' .
      '  It may need to be installed -- please refer to ' .
      l('https://pear.php.net/package/File_MARC/download', 'https://pear.php.net/package/File_MARC/download',
        array('target' => '_blank')) . '.<br>' .
      'Message : ' . $e->getMessage() . '<br>' .
      'Code : ' . $e->getCode();
    drupal_set_message($message, 'error');
    return FALSE;
  }
  return TRUE;
}
