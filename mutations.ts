/* tslint:disable */
/* eslint-disable */
// this is an auto generated file. This will be overwritten

import * as APITypes from "./API";
type GeneratedMutation<InputType, OutputType> = string & {
  __generatedMutationInput: InputType;
  __generatedMutationOutput: OutputType;
};

export const createMedicationSchedule = /* GraphQL */ `mutation CreateMedicationSchedule(
  $condition: ModelMedicationScheduleConditionInput
  $input: CreateMedicationScheduleInput!
) {
  createMedicationSchedule(condition: $condition, input: $input) {
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
` as GeneratedMutation<
  APITypes.CreateMedicationScheduleMutationVariables,
  APITypes.CreateMedicationScheduleMutation
>;
export const createUserProfile = /* GraphQL */ `mutation CreateUserProfile(
  $condition: ModelUserProfileConditionInput
  $input: CreateUserProfileInput!
) {
  createUserProfile(condition: $condition, input: $input) {
    createdAt
    email
    id
    profileOwner
    updatedAt
    __typename
  }
}
` as GeneratedMutation<
  APITypes.CreateUserProfileMutationVariables,
  APITypes.CreateUserProfileMutation
>;
export const deleteMedicationSchedule = /* GraphQL */ `mutation DeleteMedicationSchedule(
  $condition: ModelMedicationScheduleConditionInput
  $input: DeleteMedicationScheduleInput!
) {
  deleteMedicationSchedule(condition: $condition, input: $input) {
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
` as GeneratedMutation<
  APITypes.DeleteMedicationScheduleMutationVariables,
  APITypes.DeleteMedicationScheduleMutation
>;
export const deleteUserProfile = /* GraphQL */ `mutation DeleteUserProfile(
  $condition: ModelUserProfileConditionInput
  $input: DeleteUserProfileInput!
) {
  deleteUserProfile(condition: $condition, input: $input) {
    createdAt
    email
    id
    profileOwner
    updatedAt
    __typename
  }
}
` as GeneratedMutation<
  APITypes.DeleteUserProfileMutationVariables,
  APITypes.DeleteUserProfileMutation
>;
export const updateMedicationSchedule = /* GraphQL */ `mutation UpdateMedicationSchedule(
  $condition: ModelMedicationScheduleConditionInput
  $input: UpdateMedicationScheduleInput!
) {
  updateMedicationSchedule(condition: $condition, input: $input) {
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
` as GeneratedMutation<
  APITypes.UpdateMedicationScheduleMutationVariables,
  APITypes.UpdateMedicationScheduleMutation
>;
export const updateUserProfile = /* GraphQL */ `mutation UpdateUserProfile(
  $condition: ModelUserProfileConditionInput
  $input: UpdateUserProfileInput!
) {
  updateUserProfile(condition: $condition, input: $input) {
    createdAt
    email
    id
    profileOwner
    updatedAt
    __typename
  }
}
` as GeneratedMutation<
  APITypes.UpdateUserProfileMutationVariables,
  APITypes.UpdateUserProfileMutation
>;
