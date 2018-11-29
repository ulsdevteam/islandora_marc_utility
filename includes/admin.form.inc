<?php

/**
 * @file
 * Handles the display/submission of the admin settings form for this module.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_marc_utility_admin_settings(array $form, array &$form_state) {
  $form = array();
  if (user_access(ISLANDORA_MARC_UTILITY)) {
    $form['user'] = array(
        '#type' => 'item',
        '#markup' => l('Use the Islandora MARC Utility',
              'admin/islandora/tools/islandora_marc_utility'),
    );
  }
  $form['mrc_identifier_tag_xpath'] = array(
    '#title' => t('XPath to MARCXML node'),
    '#type' => 'textfield',
    '#size' => 120,
    '#attributes' => array(
      'placeholder' => MRC_XPATH_PLACEHOLDER,
    ),
    '#description' => t('This should be a legal XPath that returns the single ' .
      '"identifier" node that is to be matched with a possible object\'s MODS field value.'),
    '#default_value' => variable_get('islandora_marc_utility_mrc_identifier_tag_xpath', ''),
  );
  $form['mods_solr_fieldname'] = array(
    '#title' => t('MODS identifier Solr fieldname.'),
    '#type' => 'textfield',
    '#attributes' => array(
      'placeholder' => MODS_SOLR_FIELDNAME_PLACEHOLDER,
    ),
    '#description' => t('Solr fieldname for the MODS identifier that corresponds ' .
      'to the mrc identifier (above).'),
    '#default_value' => variable_get('islandora_marc_utility_mods_solr_fieldname', ''),
  );
  $form['temp_path'] = array(
    '#title' => t('Directory for extracting MARC xml and making MODS xml files.'),
    '#type' => 'textfield',
    '#attributes' => array(
      'placeholder' => variable_get('file_temporary_path'),
    ),
    '#description' => t('Path to extract the uploaded MARC MRC or xml files.' .
      'If empty, the site\'s ') .
      (user_access('administer site configuration') ?
      l('temporary directory', 'admin/config/media/file-system') : t('temporary directory')) .
      t(' will be used.'),
    '#default_value' => variable_get('islandora_marc_utility_temp_path', ''),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Function that sets the Drupal variables with user's input.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_marc_utility_admin_settings_submit(array $form, array &$form_state) {
  drupal_set_message(t('The settings have been updated!'));
  $id = $form_state['triggering_element']['#id'];
  switch ($id) {
    case 'edit-submit':
      // If blank, provide the value from the site configured media path.
      $use_value = ($form_state['values']['temp_path'] == '') ?
          variable_get('file_temporary_path') :
          $form_state['values']['temp_path'];
      variable_set('islandora_marc_utility_temp_path', $use_value);

      // If either of these are blank, store the value defined in the placeholder.
      $use_value = ($form_state['values']['mrc_identifier_tag_xpath'] == '') ?
          MRC_XPATH_PLACEHOLDER :
          $form_state['values']['mrc_identifier_tag_xpath'];
      variable_set('islandora_marc_utility_mrc_identifier_tag_xpath', $use_value);
      $use_value = ($form_state['values']['mods_solr_fieldname'] == '') ?
          MODS_SOLR_FIELDNAME_PLACEHOLDER :
          $form_state['values']['mods_solr_fieldname'];
      variable_set('islandora_marc_utility_mods_solr_fieldname', $use_value);
      break;
  }
}
