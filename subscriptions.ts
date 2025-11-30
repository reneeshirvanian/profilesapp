/* tslint:disable */
/* eslint-disable */
// this is an auto generated file. This will be overwritten

import * as APITypes from "./API";
type GeneratedSubscription<InputType, OutputType> = string & {
  __generatedSubscriptionInput: InputType;
  __generatedSubscriptionOutput: OutputType;
};

export const onCreateMedicationSchedule = /* GraphQL */ `subscription OnCreateMedicationSchedule(
  $filter: ModelSubscriptionMedicationScheduleFilterInput
  $profileOwner: String
) {
  onCreateMedicationSchedule(filter: $filter, profileOwner: $profileOwner) {
    createdAt
    dosage
    id
    name
    profileOwner
    time
    updatedAt
    __typename
  }
}
` as GeneratedSubscription<
  APITypes.OnCreateMedicationScheduleSubscriptionVariables,
  APITypes.OnCreateMedicationScheduleSubscription
>;
export const onCreateUserProfile = /* GraphQL */ `subscription OnCreateUserProfile(
  $filter: ModelSubscriptionUserProfileFilterInput
  $profileOwner: String
) {
  onCreateUserProfile(filter: $filter, profileOwner: $profileOwner) {
    createdAt
    email
    id
    profileOwner
    updatedAt
    __typename
  }
}
` as GeneratedSubscription<
  APITypes.OnCreateUserProfileSubscriptionVariables,
  APITypes.OnCreateUserProfileSubscription
>;
export const onDeleteMedicationSchedule = /* GraphQL */ `subscription OnDeleteMedicationSchedule(
  $filter: ModelSubscriptionMedicationScheduleFilterInput
  $profileOwner: String
) {
  onDeleteMedicationSchedule(filter: $filter, profileOwner: $profileOwner) {
    createdAt
    dosage
    id
    name
    profileOwner
    time
    updatedAt
    __typename
  }
}
` as GeneratedSubscription<
  APITypes.OnDeleteMedicationScheduleSubscriptionVariables,
  APITypes.OnDeleteMedicationScheduleSubscription
>;
export const onDeleteUserProfile = /* GraphQL */ `subscription OnDeleteUserProfile(
  $filter: ModelSubscriptionUserProfileFilterInput
  $profileOwner: String
) {
  onDeleteUserProfile(filter: $filter, profileOwner: $profileOwner) {
    createdAt
    email
    id
    profileOwner
    updatedAt
    __typename
  }
}
` as GeneratedSubscription<
  APITypes.OnDeleteUserProfileSubscriptionVariables,
  APITypes.OnDeleteUserProfileSubscription
>;
export const onUpdateMedicationSchedule = /* GraphQL */ `subscription OnUpdateMedicationSchedule(
  $filter: ModelSubscriptionMedicationScheduleFilterInput
  $profileOwner: String
) {
  onUpdateMedicationSchedule(filter: $filter, profileOwner: $profileOwner) {
    createdAt
    dosage
    id
    name
    profileOwner
    time
    updatedAt
    __typename
  }
}
` as GeneratedSubscription<
  APITypes.OnUpdateMedicationScheduleSubscriptionVariables,
  APITypes.OnUpdateMedicationScheduleSubscription
>;
export const onUpdateUserProfile = /* GraphQL */ `subscription OnUpdateUserProfile(
  $filter: ModelSubscriptionUserProfileFilterInput
  $profileOwner: String
) {
  onUpdateUserProfile(filter: $filter, profileOwner: $profileOwner) {
    createdAt
    email
    id
    profileOwner
    updatedAt
    __typename
  }
}
` as GeneratedSubscription<
  APITypes.OnUpdateUserProfileSubscriptionVariables,
  APITypes.OnUpdateUserProfileSubscription
>;
