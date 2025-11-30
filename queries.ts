/* tslint:disable */
/* eslint-disable */
// this is an auto generated file. This will be overwritten

import * as APITypes from "./API";
type GeneratedQuery<InputType, OutputType> = string & {
  __generatedQueryInput: InputType;
  __generatedQueryOutput: OutputType;
};

export const getMedicationSchedule = /* GraphQL */ `query GetMedicationSchedule($id: ID!) {
  getMedicationSchedule(id: $id) {
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
` as GeneratedQuery<
  APITypes.GetMedicationScheduleQueryVariables,
  APITypes.GetMedicationScheduleQuery
>;
export const getUserProfile = /* GraphQL */ `query GetUserProfile($id: ID!) {
  getUserProfile(id: $id) {
    createdAt
    email
    id
    profileOwner
    updatedAt
    __typename
  }
}
` as GeneratedQuery<
  APITypes.GetUserProfileQueryVariables,
  APITypes.GetUserProfileQuery
>;
export const listMedicationSchedules = /* GraphQL */ `query ListMedicationSchedules(
  $filter: ModelMedicationScheduleFilterInput
  $limit: Int
  $nextToken: String
) {
  listMedicationSchedules(
    filter: $filter
    limit: $limit
    nextToken: $nextToken
  ) {
    items {
      createdAt
      dosage
      id
      name
      profileOwner
      time
      updatedAt
      __typename
    }
    nextToken
    __typename
  }
}
` as GeneratedQuery<
  APITypes.ListMedicationSchedulesQueryVariables,
  APITypes.ListMedicationSchedulesQuery
>;
export const listUserProfiles = /* GraphQL */ `query ListUserProfiles(
  $filter: ModelUserProfileFilterInput
  $limit: Int
  $nextToken: String
) {
  listUserProfiles(filter: $filter, limit: $limit, nextToken: $nextToken) {
    items {
      createdAt
      email
      id
      profileOwner
      updatedAt
      __typename
    }
    nextToken
    __typename
  }
}
` as GeneratedQuery<
  APITypes.ListUserProfilesQueryVariables,
  APITypes.ListUserProfilesQuery
>;
