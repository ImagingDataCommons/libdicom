
/*
 * Implementation of Part 6 of the DICOM standard: Data Dictionary.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dicom.h"


struct dcm_Attribute {
    uint32_t tag;
    const char vr[3];
    const char keyword[63];

};



static const struct dcm_Attribute dictionary[] = {
{0X00000000, "UL", "CommandGroupLength"},
{0X00000001, "UL", "CommandLengthToEnd"},
{0X00000002, "UI", "AffectedSOPClassUID"},
{0X00000003, "UI", "RequestedSOPClassUID"},
{0X00000010, "SH", "CommandRecognitionCode"},
{0X00000100, "US", "CommandField"},
{0X00000110, "US", "MessageID"},
{0X00000120, "US", "MessageIDBeingRespondedTo"},
{0X00000200, "AE", "Initiator"},
{0X00000300, "AE", "Receiver"},
{0X00000400, "AE", "FindLocation"},
{0X00000600, "AE", "MoveDestination"},
{0X00000700, "US", "Priority"},
{0X00000800, "US", "CommandDataSetType"},
{0X00000850, "US", "NumberOfMatches"},
{0X00000860, "US", "ResponseSequenceNumber"},
{0X00000900, "US", "Status"},
{0X00000901, "AT", "OffendingElement"},
{0X00000902, "LO", "ErrorComment"},
{0X00000903, "US", "ErrorID"},
{0X00001000, "UI", "AffectedSOPInstanceUID"},
{0X00001001, "UI", "RequestedSOPInstanceUID"},
{0X00001002, "US", "EventTypeID"},
{0X00001005, "AT", "AttributeIdentifierList"},
{0X00001008, "US", "ActionTypeID"},
{0X00001020, "US", "NumberOfRemainingSuboperations"},
{0X00001021, "US", "NumberOfCompletedSuboperations"},
{0X00001022, "US", "NumberOfFailedSuboperations"},
{0X00001023, "US", "NumberOfWarningSuboperations"},
{0X00001030, "AE", "MoveOriginatorApplicationEntityTitle"},
{0X00001031, "US", "MoveOriginatorMessageID"},
{0X00004000, "LT", "DialogReceiver"},
{0X00004010, "LT", "TerminalType"},
{0X00005010, "SH", "MessageSetID"},
{0X00005020, "SH", "EndMessageID"},
{0X00005110, "LT", "DisplayFormat"},
{0X00005120, "LT", "PagePositionID"},
{0X00005130, "CS", "TextFormatID"},
{0X00005140, "CS", "NormalReverse"},
{0X00005150, "CS", "AddGrayScale"},
{0X00005160, "CS", "Borders"},
{0X00005170, "IS", "Copies"},
{0X00005180, "CS", "CommandMagnificationType"},
{0X00005190, "CS", "Erase"},
{0X000051A0, "CS", "Print"},
{0X000051B0, "US", "Overlays"},
{0X00020000, "UL", "FileMetaInformationGroupLength"},
{0X00020001, "OB", "FileMetaInformationVersion"},
{0X00020002, "UI", "MediaStorageSOPClassUID"},
{0X00020003, "UI", "MediaStorageSOPInstanceUID"},
{0X00020010, "UI", "TransferSyntaxUID"},
{0X00020012, "UI", "ImplementationClassUID"},
{0X00020013, "SH", "ImplementationVersionName"},
{0X00020016, "AE", "SourceApplicationEntityTitle"},
{0X00020017, "AE", "SendingApplicationEntityTitle"},
{0X00020018, "AE", "ReceivingApplicationEntityTitle"},
{0X00020026, "UR", "SourcePresentationAddress"},
{0X00020027, "UR", "SendingPresentationAddress"},
{0X00020028, "UR", "ReceivingPresentationAddress"},
{0X00020031, "OB", "RTVMetaInformationVersion"},
{0X00020032, "UI", "RTVCommunicationSOPClassUID"},
{0X00020033, "UI", "RTVCommunicationSOPInstanceUID"},
{0X00020035, "OB", "RTVSourceIdentifier"},
{0X00020036, "OB", "RTVFlowIdentifier"},
{0X00020037, "UL", "RTVFlowRTPSamplingRate"},
{0X00020038, "FD", "RTVFlowActualFrameDuration"},
{0X00020100, "UI", "PrivateInformationCreatorUID"},
{0X00020102, "OB", "PrivateInformation"},
{0X00041130, "CS", "FileSetID"},
{0X00041141, "CS", "FileSetDescriptorFileID"},
{0X00041142, "CS", "SpecificCharacterSetOfFileSetDescriptorFile"},
{0X00041200, "UL", "OffsetOfTheFirstDirectoryRecordOfTheRootDirectoryEntity"},
{0X00041202, "UL", "OffsetOfTheLastDirectoryRecordOfTheRootDirectoryEntity"},
{0X00041212, "US", "FileSetConsistencyFlag"},
{0X00041220, "SQ", "DirectoryRecordSequence"},
{0X00041400, "UL", "OffsetOfTheNextDirectoryRecord"},
{0X00041410, "US", "RecordInUseFlag"},
{0X00041420, "UL", "OffsetOfReferencedLowerLevelDirectoryEntity"},
{0X00041430, "CS", "DirectoryRecordType"},
{0X00041432, "UI", "PrivateRecordUID"},
{0X00041500, "CS", "ReferencedFileID"},
{0X00041504, "UL", "MRDRDirectoryRecordOffset"},
{0X00041510, "UI", "ReferencedSOPClassUIDInFile"},
{0X00041511, "UI", "ReferencedSOPInstanceUIDInFile"},
{0X00041512, "UI", "ReferencedTransferSyntaxUIDInFile"},
{0X0004151A, "UI", "ReferencedRelatedGeneralSOPClassUIDInFile"},
{0X00041600, "UL", "NumberOfReferences"},
{0X00080001, "UL", "LengthToEnd"},
{0X00080005, "CS", "SpecificCharacterSet"},
{0X00080006, "SQ", "LanguageCodeSequence"},
{0X00080008, "CS", "ImageType"},
{0X00080010, "SH", "RecognitionCode"},
{0X00080012, "DA", "InstanceCreationDate"},
{0X00080013, "TM", "InstanceCreationTime"},
{0X00080014, "UI", "InstanceCreatorUID"},
{0X00080015, "DT", "InstanceCoercionDateTime"},
{0X00080016, "UI", "SOPClassUID"},
{0X00080018, "UI", "SOPInstanceUID"},
{0X0008001A, "UI", "RelatedGeneralSOPClassUID"},
{0X0008001B, "UI", "OriginalSpecializedSOPClassUID"},
{0X00080020, "DA", "StudyDate"},
{0X00080021, "DA", "SeriesDate"},
{0X00080022, "DA", "AcquisitionDate"},
{0X00080023, "DA", "ContentDate"},
{0X00080024, "DA", "OverlayDate"},
{0X00080025, "DA", "CurveDate"},
{0X0008002A, "DT", "AcquisitionDateTime"},
{0X00080030, "TM", "StudyTime"},
{0X00080031, "TM", "SeriesTime"},
{0X00080032, "TM", "AcquisitionTime"},
{0X00080033, "TM", "ContentTime"},
{0X00080034, "TM", "OverlayTime"},
{0X00080035, "TM", "CurveTime"},
{0X00080040, "US", "DataSetType"},
{0X00080041, "LO", "DataSetSubtype"},
{0X00080042, "CS", "NuclearMedicineSeriesType"},
{0X00080050, "SH", "AccessionNumber"},
{0X00080051, "SQ", "IssuerOfAccessionNumberSequence"},
{0X00080052, "CS", "QueryRetrieveLevel"},
{0X00080053, "CS", "QueryRetrieveView"},
{0X00080054, "AE", "RetrieveAETitle"},
{0X00080055, "AE", "StationAETitle"},
{0X00080056, "CS", "InstanceAvailability"},
{0X00080058, "UI", "FailedSOPInstanceUIDList"},
{0X00080060, "CS", "Modality"},
{0X00080061, "CS", "ModalitiesInStudy"},
{0X00080062, "UI", "SOPClassesInStudy"},
{0X00080063, "SQ", "AnatomicRegionsInStudyCodeSequence"},
{0X00080064, "CS", "ConversionType"},
{0X00080068, "CS", "PresentationIntentType"},
{0X00080070, "LO", "Manufacturer"},
{0X00080080, "LO", "InstitutionName"},
{0X00080081, "ST", "InstitutionAddress"},
{0X00080082, "SQ", "InstitutionCodeSequence"},
{0X00080090, "PN", "ReferringPhysicianName"},
{0X00080092, "ST", "ReferringPhysicianAddress"},
{0X00080094, "SH", "ReferringPhysicianTelephoneNumbers"},
{0X00080096, "SQ", "ReferringPhysicianIdentificationSequence"},
{0X0008009C, "PN", "ConsultingPhysicianName"},
{0X0008009D, "SQ", "ConsultingPhysicianIdentificationSequence"},
{0X00080100, "SH", "CodeValue"},
{0X00080101, "LO", "ExtendedCodeValue"},
{0X00080102, "SH", "CodingSchemeDesignator"},
{0X00080103, "SH", "CodingSchemeVersion"},
{0X00080104, "LO", "CodeMeaning"},
{0X00080105, "CS", "MappingResource"},
{0X00080106, "DT", "ContextGroupVersion"},
{0X00080107, "DT", "ContextGroupLocalVersion"},
{0X00080108, "LT", "ExtendedCodeMeaning"},
{0X00080109, "SQ", "CodingSchemeResourcesSequence"},
{0X0008010A, "CS", "CodingSchemeURLType"},
{0X0008010B, "CS", "ContextGroupExtensionFlag"},
{0X0008010C, "UI", "CodingSchemeUID"},
{0X0008010D, "UI", "ContextGroupExtensionCreatorUID"},
{0X0008010E, "UR", "CodingSchemeURL"},
{0X0008010F, "CS", "ContextIdentifier"},
{0X00080110, "SQ", "CodingSchemeIdentificationSequence"},
{0X00080112, "LO", "CodingSchemeRegistry"},
{0X00080114, "ST", "CodingSchemeExternalID"},
{0X00080115, "ST", "CodingSchemeName"},
{0X00080116, "ST", "CodingSchemeResponsibleOrganization"},
{0X00080117, "UI", "ContextUID"},
{0X00080118, "UI", "MappingResourceUID"},
{0X00080119, "UC", "LongCodeValue"},
{0X00080120, "UR", "URNCodeValue"},
{0X00080121, "SQ", "EquivalentCodeSequence"},
{0X00080122, "LO", "MappingResourceName"},
{0X00080123, "SQ", "ContextGroupIdentificationSequence"},
{0X00080124, "SQ", "MappingResourceIdentificationSequence"},
{0X00080201, "SH", "TimezoneOffsetFromUTC"},
{0X00080202, "OB", ""},
{0X00080220, "SQ", "ResponsibleGroupCodeSequence"},
{0X00080221, "CS", "EquipmentModality"},
{0X00080222, "LO", "ManufacturerRelatedModelGroup"},
{0X00080300, "SQ", "PrivateDataElementCharacteristicsSequence"},
{0X00080301, "US", "PrivateGroupReference"},
{0X00080302, "LO", "PrivateCreatorReference"},
{0X00080303, "CS", "BlockIdentifyingInformationStatus"},
{0X00080304, "US", "NonidentifyingPrivateElements"},
{0X00080305, "SQ", "DeidentificationActionSequence"},
{0X00080306, "US", "IdentifyingPrivateElements"},
{0X00080307, "CS", "DeidentificationAction"},
{0X00080308, "US", "PrivateDataElement"},
{0X00080309, "UL", "PrivateDataElementValueMultiplicity"},
{0X0008030A, "CS", "PrivateDataElementValueRepresentation"},
{0X0008030B, "UL", "PrivateDataElementNumberOfItems"},
{0X0008030C, "UC", "PrivateDataElementName"},
{0X0008030D, "UC", "PrivateDataElementKeyword"},
{0X0008030E, "UT", "PrivateDataElementDescription"},
{0X0008030F, "UT", "PrivateDataElementEncoding"},
{0X00080310, "SQ", "PrivateDataElementDefinitionSequence"},
{0X00081000, "AE", "NetworkID"},
{0X00081010, "SH", "StationName"},
{0X00081030, "LO", "StudyDescription"},
{0X00081032, "SQ", "ProcedureCodeSequence"},
{0X0008103E, "LO", "SeriesDescription"},
{0X0008103F, "SQ", "SeriesDescriptionCodeSequence"},
{0X00081040, "LO", "InstitutionalDepartmentName"},
{0X00081041, "SQ", "InstitutionalDepartmentTypeCodeSequence"},
{0X00081048, "PN", "PhysiciansOfRecord"},
{0X00081049, "SQ", "PhysiciansOfRecordIdentificationSequence"},
{0X00081050, "PN", "PerformingPhysicianName"},
{0X00081052, "SQ", "PerformingPhysicianIdentificationSequence"},
{0X00081060, "PN", "NameOfPhysiciansReadingStudy"},
{0X00081062, "SQ", "PhysiciansReadingStudyIdentificationSequence"},
{0X00081070, "PN", "OperatorsName"},
{0X00081072, "SQ", "OperatorIdentificationSequence"},
{0X00081080, "LO", "AdmittingDiagnosesDescription"},
{0X00081084, "SQ", "AdmittingDiagnosesCodeSequence"},
{0X00081090, "LO", "ManufacturerModelName"},
{0X00081100, "SQ", "ReferencedResultsSequence"},
{0X00081110, "SQ", "ReferencedStudySequence"},
{0X00081111, "SQ", "ReferencedPerformedProcedureStepSequence"},
{0X00081115, "SQ", "ReferencedSeriesSequence"},
{0X00081120, "SQ", "ReferencedPatientSequence"},
{0X00081125, "SQ", "ReferencedVisitSequence"},
{0X00081130, "SQ", "ReferencedOverlaySequence"},
{0X00081134, "SQ", "ReferencedStereometricInstanceSequence"},
{0X0008113A, "SQ", "ReferencedWaveformSequence"},
{0X00081140, "SQ", "ReferencedImageSequence"},
{0X00081145, "SQ", "ReferencedCurveSequence"},
{0X0008114A, "SQ", "ReferencedInstanceSequence"},
{0X0008114B, "SQ", "ReferencedRealWorldValueMappingInstanceSequence"},
{0X00081150, "UI", "ReferencedSOPClassUID"},
{0X00081155, "UI", "ReferencedSOPInstanceUID"},
{0X00081156, "SQ", "DefinitionSourceSequence"},
{0X0008115A, "UI", "SOPClassesSupported"},
{0X00081160, "IS", "ReferencedFrameNumber"},
{0X00081161, "UL", "SimpleFrameList"},
{0X00081162, "UL", "CalculatedFrameList"},
{0X00081163, "FD", "TimeRange"},
{0X00081164, "SQ", "FrameExtractionSequence"},
{0X00081167, "UI", "MultiFrameSourceSOPInstanceUID"},
{0X00081190, "UR", "RetrieveURL"},
{0X00081195, "UI", "TransactionUID"},
{0X00081196, "US", "WarningReason"},
{0X00081197, "US", "FailureReason"},
{0X00081198, "SQ", "FailedSOPSequence"},
{0X00081199, "SQ", "ReferencedSOPSequence"},
{0X0008119A, "SQ", "OtherFailuresSequence"},
{0X00081200, "SQ", "StudiesContainingOtherReferencedInstancesSequence"},
{0X00081250, "SQ", "RelatedSeriesSequence"},
{0X00082110, "CS", "LossyImageCompressionRetired"},
{0X00082111, "ST", "DerivationDescription"},
{0X00082112, "SQ", "SourceImageSequence"},
{0X00082120, "SH", "StageName"},
{0X00082122, "IS", "StageNumber"},
{0X00082124, "IS", "NumberOfStages"},
{0X00082127, "SH", "ViewName"},
{0X00082128, "IS", "ViewNumber"},
{0X00082129, "IS", "NumberOfEventTimers"},
{0X0008212A, "IS", "NumberOfViewsInStage"},
{0X00082130, "DS", "EventElapsedTimes"},
{0X00082132, "LO", "EventTimerNames"},
{0X00082133, "SQ", "EventTimerSequence"},
{0X00082134, "FD", "EventTimeOffset"},
{0X00082135, "SQ", "EventCodeSequence"},
{0X00082142, "IS", "StartTrim"},
{0X00082143, "IS", "StopTrim"},
{0X00082144, "IS", "RecommendedDisplayFrameRate"},
{0X00082200, "CS", "TransducerPosition"},
{0X00082204, "CS", "TransducerOrientation"},
{0X00082208, "CS", "AnatomicStructure"},
{0X00082218, "SQ", "AnatomicRegionSequence"},
{0X00082220, "SQ", "AnatomicRegionModifierSequence"},
{0X00082228, "SQ", "PrimaryAnatomicStructureSequence"},
{0X00082229, "SQ", "AnatomicStructureSpaceOrRegionSequence"},
{0X00082230, "SQ", "PrimaryAnatomicStructureModifierSequence"},
{0X00082240, "SQ", "TransducerPositionSequence"},
{0X00082242, "SQ", "TransducerPositionModifierSequence"},
{0X00082244, "SQ", "TransducerOrientationSequence"},
{0X00082246, "SQ", "TransducerOrientationModifierSequence"},
{0X00082251, "SQ", "AnatomicStructureSpaceOrRegionCodeSequenceTrial"},
{0X00082253, "SQ", "AnatomicPortalOfEntranceCodeSequenceTrial"},
{0X00082255, "SQ", "AnatomicApproachDirectionCodeSequenceTrial"},
{0X00082256, "ST", "AnatomicPerspectiveDescriptionTrial"},
{0X00082257, "SQ", "AnatomicPerspectiveCodeSequenceTrial"},
{0X00082258, "ST", "AnatomicLocationOfExaminingInstrumentDescriptionTrial"},
{0X00082259, "SQ", "AnatomicLocationOfExaminingInstrumentCodeSequenceTrial"},
{0X0008225A, "SQ", "AnatomicStructureSpaceOrRegionModifierCodeSequenceTrial"},
{0X0008225C, "SQ", "OnAxisBackgroundAnatomicStructureCodeSequenceTrial"},
{0X00083001, "SQ", "AlternateRepresentationSequence"},
{0X00083002, "UI", "AvailableTransferSyntaxUID"},
{0X00083010, "UI", "IrradiationEventUID"},
{0X00083011, "SQ", "SourceIrradiationEventSequence"},
{0X00083012, "UI", "RadiopharmaceuticalAdministrationEventUID"},
{0X00084000, "LT", "IdentifyingComments"},
{0X00089007, "CS", "FrameType"},
{0X00089092, "SQ", "ReferencedImageEvidenceSequence"},
{0X00089121, "SQ", "ReferencedRawDataSequence"},
{0X00089123, "UI", "CreatorVersionUID"},
{0X00089124, "SQ", "DerivationImageSequence"},
{0X00089154, "SQ", "SourceImageEvidenceSequence"},
{0X00089205, "CS", "PixelPresentation"},
{0X00089206, "CS", "VolumetricProperties"},
{0X00089207, "CS", "VolumeBasedCalculationTechnique"},
{0X00089208, "CS", "ComplexImageComponent"},
{0X00089209, "CS", "AcquisitionContrast"},
{0X00089215, "SQ", "DerivationCodeSequence"},
{0X00089237, "SQ", "ReferencedPresentationStateSequence"},
{0X00089410, "SQ", "ReferencedOtherPlaneSequence"},
{0X00089458, "SQ", "FrameDisplaySequence"},
{0X00089459, "FL", "RecommendedDisplayFrameRateInFloat"},
{0X00089460, "CS", "SkipFrameRangeFlag"},
{0X00100010, "PN", "PatientName"},
{0X00100020, "LO", "PatientID"},
{0X00100021, "LO", "IssuerOfPatientID"},
{0X00100022, "CS", "TypeOfPatientID"},
{0X00100024, "SQ", "IssuerOfPatientIDQualifiersSequence"},
{0X00100026, "SQ", "SourcePatientGroupIdentificationSequence"},
{0X00100027, "SQ", "GroupOfPatientsIdentificationSequence"},
{0X00100028, "US", "SubjectRelativePositionInImage"},
{0X00100030, "DA", "PatientBirthDate"},
{0X00100032, "TM", "PatientBirthTime"},
{0X00100033, "LO", "PatientBirthDateInAlternativeCalendar"},
{0X00100034, "LO", "PatientDeathDateInAlternativeCalendar"},
{0X00100035, "CS", "PatientAlternativeCalendar"},
{0X00100040, "CS", "PatientSex"},
{0X00100050, "SQ", "PatientInsurancePlanCodeSequence"},
{0X00100101, "SQ", "PatientPrimaryLanguageCodeSequence"},
{0X00100102, "SQ", "PatientPrimaryLanguageModifierCodeSequence"},
{0X00100200, "CS", "QualityControlSubject"},
{0X00100201, "SQ", "QualityControlSubjectTypeCodeSequence"},
{0X00100212, "UC", "StrainDescription"},
{0X00100213, "LO", "StrainNomenclature"},
{0X00100214, "LO", "StrainStockNumber"},
{0X00100215, "SQ", "StrainSourceRegistryCodeSequence"},
{0X00100216, "SQ", "StrainStockSequence"},
{0X00100217, "LO", "StrainSource"},
{0X00100218, "UT", "StrainAdditionalInformation"},
{0X00100219, "SQ", "StrainCodeSequence"},
{0X00100221, "SQ", "GeneticModificationsSequence"},
{0X00100222, "UC", "GeneticModificationsDescription"},
{0X00100223, "LO", "GeneticModificationsNomenclature"},
{0X00100229, "SQ", "GeneticModificationsCodeSequence"},
{0X00101000, "LO", "OtherPatientIDs"},
{0X00101001, "PN", "OtherPatientNames"},
{0X00101002, "SQ", "OtherPatientIDsSequence"},
{0X00101005, "PN", "PatientBirthName"},
{0X00101010, "AS", "PatientAge"},
{0X00101020, "DS", "PatientSize"},
{0X00101021, "SQ", "PatientSizeCodeSequence"},
{0X00101022, "DS", "PatientBodyMassIndex"},
{0X00101023, "DS", "MeasuredAPDimension"},
{0X00101024, "DS", "MeasuredLateralDimension"},
{0X00101030, "DS", "PatientWeight"},
{0X00101040, "LO", "PatientAddress"},
{0X00101050, "LO", "InsurancePlanIdentification"},
{0X00101060, "PN", "PatientMotherBirthName"},
{0X00101080, "LO", "MilitaryRank"},
{0X00101081, "LO", "BranchOfService"},
{0X00101090, "LO", "MedicalRecordLocator"},
{0X00101100, "SQ", "ReferencedPatientPhotoSequence"},
{0X00102000, "LO", "MedicalAlerts"},
{0X00102110, "LO", "Allergies"},
{0X00102150, "LO", "CountryOfResidence"},
{0X00102152, "LO", "RegionOfResidence"},
{0X00102154, "SH", "PatientTelephoneNumbers"},
{0X00102155, "LT", "PatientTelecomInformation"},
{0X00102160, "SH", "EthnicGroup"},
{0X00102180, "SH", "Occupation"},
{0X001021A0, "CS", "SmokingStatus"},
{0X001021B0, "LT", "AdditionalPatientHistory"},
{0X001021C0, "US", "PregnancyStatus"},
{0X001021D0, "DA", "LastMenstrualDate"},
{0X001021F0, "LO", "PatientReligiousPreference"},
{0X00102201, "LO", "PatientSpeciesDescription"},
{0X00102202, "SQ", "PatientSpeciesCodeSequence"},
{0X00102203, "CS", "PatientSexNeutered"},
{0X00102210, "CS", "AnatomicalOrientationType"},
{0X00102292, "LO", "PatientBreedDescription"},
{0X00102293, "SQ", "PatientBreedCodeSequence"},
{0X00102294, "SQ", "BreedRegistrationSequence"},
{0X00102295, "LO", "BreedRegistrationNumber"},
{0X00102296, "SQ", "BreedRegistryCodeSequence"},
{0X00102297, "PN", "ResponsiblePerson"},
{0X00102298, "CS", "ResponsiblePersonRole"},
{0X00102299, "LO", "ResponsibleOrganization"},
{0X00104000, "LT", "PatientComments"},
{0X00109431, "FL", "ExaminedBodyThickness"},
{0X00120010, "LO", "ClinicalTrialSponsorName"},
{0X00120020, "LO", "ClinicalTrialProtocolID"},
{0X00120021, "LO", "ClinicalTrialProtocolName"},
{0X00120030, "LO", "ClinicalTrialSiteID"},
{0X00120031, "LO", "ClinicalTrialSiteName"},
{0X00120040, "LO", "ClinicalTrialSubjectID"},
{0X00120042, "LO", "ClinicalTrialSubjectReadingID"},
{0X00120050, "LO", "ClinicalTrialTimePointID"},
{0X00120051, "ST", "ClinicalTrialTimePointDescription"},
{0X00120052, "FD", "LongitudinalTemporalOffsetFromEvent"},
{0X00120053, "CS", "LongitudinalTemporalEventType"},
{0X00120060, "LO", "ClinicalTrialCoordinatingCenterName"},
{0X00120062, "CS", "PatientIdentityRemoved"},
{0X00120063, "LO", "DeidentificationMethod"},
{0X00120064, "SQ", "DeidentificationMethodCodeSequence"},
{0X00120071, "LO", "ClinicalTrialSeriesID"},
{0X00120072, "LO", "ClinicalTrialSeriesDescription"},
{0X00120081, "LO", "ClinicalTrialProtocolEthicsCommitteeName"},
{0X00120082, "LO", "ClinicalTrialProtocolEthicsCommitteeApprovalNumber"},
{0X00120083, "SQ", "ConsentForClinicalTrialUseSequence"},
{0X00120084, "CS", "DistributionType"},
{0X00120085, "CS", "ConsentForDistributionFlag"},
{0X00120086, "DA", "EthicsCommitteeApprovalEffectivenessStartDate"},
{0X00120087, "DA", "EthicsCommitteeApprovalEffectivenessEndDate"},
{0X00140023, "ST", "CADFileFormat"},
{0X00140024, "ST", "ComponentReferenceSystem"},
{0X00140025, "ST", "ComponentManufacturingProcedure"},
{0X00140028, "ST", "ComponentManufacturer"},
{0X00140030, "DS", "MaterialThickness"},
{0X00140032, "DS", "MaterialPipeDiameter"},
{0X00140034, "DS", "MaterialIsolationDiameter"},
{0X00140042, "ST", "MaterialGrade"},
{0X00140044, "ST", "MaterialPropertiesDescription"},
{0X00140045, "ST", "MaterialPropertiesFileFormatRetired"},
{0X00140046, "LT", "MaterialNotes"},
{0X00140050, "CS", "ComponentShape"},
{0X00140052, "CS", "CurvatureType"},
{0X00140054, "DS", "OuterDiameter"},
{0X00140056, "DS", "InnerDiameter"},
{0X00140100, "LO", "ComponentWelderIDs"},
{0X00140101, "CS", "SecondaryApprovalStatus"},
{0X00140102, "DA", "SecondaryReviewDate"},
{0X00140103, "TM", "SecondaryReviewTime"},
{0X00140104, "PN", "SecondaryReviewerName"},
{0X00140105, "ST", "RepairID"},
{0X00140106, "SQ", "MultipleComponentApprovalSequence"},
{0X00140107, "CS", "OtherApprovalStatus"},
{0X00140108, "CS", "OtherSecondaryApprovalStatus"},
{0X00141010, "ST", "ActualEnvironmentalConditions"},
{0X00141020, "DA", "ExpiryDate"},
{0X00141040, "ST", "EnvironmentalConditions"},
{0X00142002, "SQ", "EvaluatorSequence"},
{0X00142004, "IS", "EvaluatorNumber"},
{0X00142006, "PN", "EvaluatorName"},
{0X00142008, "IS", "EvaluationAttempt"},
{0X00142012, "SQ", "IndicationSequence"},
{0X00142014, "IS", "IndicationNumber"},
{0X00142016, "SH", "IndicationLabel"},
{0X00142018, "ST", "IndicationDescription"},
{0X0014201A, "CS", "IndicationType"},
{0X0014201C, "CS", "IndicationDisposition"},
{0X0014201E, "SQ", "IndicationROISequence"},
{0X00142030, "SQ", "IndicationPhysicalPropertySequence"},
{0X00142032, "SH", "PropertyLabel"},
{0X00142202, "IS", "CoordinateSystemNumberOfAxes"},
{0X00142204, "SQ", "CoordinateSystemAxesSequence"},
{0X00142206, "ST", "CoordinateSystemAxisDescription"},
{0X00142208, "CS", "CoordinateSystemDataSetMapping"},
{0X0014220A, "IS", "CoordinateSystemAxisNumber"},
{0X0014220C, "CS", "CoordinateSystemAxisType"},
{0X0014220E, "CS", "CoordinateSystemAxisUnits"},
{0X00142210, "OB", "CoordinateSystemAxisValues"},
{0X00142220, "SQ", "CoordinateSystemTransformSequence"},
{0X00142222, "ST", "TransformDescription"},
{0X00142224, "IS", "TransformNumberOfAxes"},
{0X00142226, "IS", "TransformOrderOfAxes"},
{0X00142228, "CS", "TransformedAxisUnits"},
{0X0014222A, "DS", "CoordinateSystemTransformRotationAndScaleMatrix"},
{0X0014222C, "DS", "CoordinateSystemTransformTranslationMatrix"},
{0X00143011, "DS", "InternalDetectorFrameTime"},
{0X00143012, "DS", "NumberOfFramesIntegrated"},
{0X00143020, "SQ", "DetectorTemperatureSequence"},
{0X00143022, "ST", "SensorName"},
{0X00143024, "DS", "HorizontalOffsetOfSensor"},
{0X00143026, "DS", "VerticalOffsetOfSensor"},
{0X00143028, "DS", "SensorTemperature"},
{0X00143040, "SQ", "DarkCurrentSequence"},
{0X00143050, "OW", "DarkCurrentCounts"},
{0X00143060, "SQ", "GainCorrectionReferenceSequence"},
{0X00143070, "OW", "AirCounts"},
{0X00143071, "DS", "KVUsedInGainCalibration"},
{0X00143072, "DS", "MAUsedInGainCalibration"},
{0X00143073, "DS", "NumberOfFramesUsedForIntegration"},
{0X00143074, "LO", "FilterMaterialUsedInGainCalibration"},
{0X00143075, "DS", "FilterThicknessUsedInGainCalibration"},
{0X00143076, "DA", "DateOfGainCalibration"},
{0X00143077, "TM", "TimeOfGainCalibration"},
{0X00143080, "OB", "BadPixelImage"},
{0X00143099, "LT", "CalibrationNotes"},
{0X00143100, "LT", "LinearityCorrectionTechnique"},
{0X00143101, "LT", "BeamHardeningCorrectionTechnique"},
{0X00144002, "SQ", "PulserEquipmentSequence"},
{0X00144004, "CS", "PulserType"},
{0X00144006, "LT", "PulserNotes"},
{0X00144008, "SQ", "ReceiverEquipmentSequence"},
{0X0014400A, "CS", "AmplifierType"},
{0X0014400C, "LT", "ReceiverNotes"},
{0X0014400E, "SQ", "PreAmplifierEquipmentSequence"},
{0X0014400F, "LT", "PreAmplifierNotes"},
{0X00144010, "SQ", "TransmitTransducerSequence"},
{0X00144011, "SQ", "ReceiveTransducerSequence"},
{0X00144012, "US", "NumberOfElements"},
{0X00144013, "CS", "ElementShape"},
{0X00144014, "DS", "ElementDimensionA"},
{0X00144015, "DS", "ElementDimensionB"},
{0X00144016, "DS", "ElementPitchA"},
{0X00144017, "DS", "MeasuredBeamDimensionA"},
{0X00144018, "DS", "MeasuredBeamDimensionB"},
{0X00144019, "DS", "LocationOfMeasuredBeamDiameter"},
{0X0014401A, "DS", "NominalFrequency"},
{0X0014401B, "DS", "MeasuredCenterFrequency"},
{0X0014401C, "DS", "MeasuredBandwidth"},
{0X0014401D, "DS", "ElementPitchB"},
{0X00144020, "SQ", "PulserSettingsSequence"},
{0X00144022, "DS", "PulseWidth"},
{0X00144024, "DS", "ExcitationFrequency"},
{0X00144026, "CS", "ModulationType"},
{0X00144028, "DS", "Damping"},
{0X00144030, "SQ", "ReceiverSettingsSequence"},
{0X00144031, "DS", "AcquiredSoundpathLength"},
{0X00144032, "CS", "AcquisitionCompressionType"},
{0X00144033, "IS", "AcquisitionSampleSize"},
{0X00144034, "DS", "RectifierSmoothing"},
{0X00144035, "SQ", "DACSequence"},
{0X00144036, "CS", "DACType"},
{0X00144038, "DS", "DACGainPoints"},
{0X0014403A, "DS", "DACTimePoints"},
{0X0014403C, "DS", "DACAmplitude"},
{0X00144040, "SQ", "PreAmplifierSettingsSequence"},
{0X00144050, "SQ", "TransmitTransducerSettingsSequence"},
{0X00144051, "SQ", "ReceiveTransducerSettingsSequence"},
{0X00144052, "DS", "IncidentAngle"},
{0X00144054, "ST", "CouplingTechnique"},
{0X00144056, "ST", "CouplingMedium"},
{0X00144057, "DS", "CouplingVelocity"},
{0X00144058, "DS", "ProbeCenterLocationX"},
{0X00144059, "DS", "ProbeCenterLocationZ"},
{0X0014405A, "DS", "SoundPathLength"},
{0X0014405C, "ST", "DelayLawIdentifier"},
{0X00144060, "SQ", "GateSettingsSequence"},
{0X00144062, "DS", "GateThreshold"},
{0X00144064, "DS", "VelocityOfSound"},
{0X00144070, "SQ", "CalibrationSettingsSequence"},
{0X00144072, "ST", "CalibrationProcedure"},
{0X00144074, "SH", "ProcedureVersion"},
{0X00144076, "DA", "ProcedureCreationDate"},
{0X00144078, "DA", "ProcedureExpirationDate"},
{0X0014407A, "DA", "ProcedureLastModifiedDate"},
{0X0014407C, "TM", "CalibrationTime"},
{0X0014407E, "DA", "CalibrationDate"},
{0X00144080, "SQ", "ProbeDriveEquipmentSequence"},
{0X00144081, "CS", "DriveType"},
{0X00144082, "LT", "ProbeDriveNotes"},
{0X00144083, "SQ", "DriveProbeSequence"},
{0X00144084, "DS", "ProbeInductance"},
{0X00144085, "DS", "ProbeResistance"},
{0X00144086, "SQ", "ReceiveProbeSequence"},
{0X00144087, "SQ", "ProbeDriveSettingsSequence"},
{0X00144088, "DS", "BridgeResistors"},
{0X00144089, "DS", "ProbeOrientationAngle"},
{0X0014408B, "DS", "UserSelectedGainY"},
{0X0014408C, "DS", "UserSelectedPhase"},
{0X0014408D, "DS", "UserSelectedOffsetX"},
{0X0014408E, "DS", "UserSelectedOffsetY"},
{0X00144091, "SQ", "ChannelSettingsSequence"},
{0X00144092, "DS", "ChannelThreshold"},
{0X0014409A, "SQ", "ScannerSettingsSequence"},
{0X0014409B, "ST", "ScanProcedure"},
{0X0014409C, "DS", "TranslationRateX"},
{0X0014409D, "DS", "TranslationRateY"},
{0X0014409F, "DS", "ChannelOverlap"},
{0X001440A0, "LO", "ImageQualityIndicatorType"},
{0X001440A1, "LO", "ImageQualityIndicatorMaterial"},
{0X001440A2, "LO", "ImageQualityIndicatorSize"},
{0X00145002, "IS", "LINACEnergy"},
{0X00145004, "IS", "LINACOutput"},
{0X00145100, "US", "ActiveAperture"},
{0X00145101, "DS", "TotalAperture"},
{0X00145102, "DS", "ApertureElevation"},
{0X00145103, "DS", "MainLobeAngle"},
{0X00145104, "DS", "MainRoofAngle"},
{0X00145105, "CS", "ConnectorType"},
{0X00145106, "SH", "WedgeModelNumber"},
{0X00145107, "DS", "WedgeAngleFloat"},
{0X00145108, "DS", "WedgeRoofAngle"},
{0X00145109, "CS", "WedgeElement1Position"},
{0X0014510A, "DS", "WedgeMaterialVelocity"},
{0X0014510B, "SH", "WedgeMaterial"},
{0X0014510C, "DS", "WedgeOffsetZ"},
{0X0014510D, "DS", "WedgeOriginOffsetX"},
{0X0014510E, "DS", "WedgeTimeDelay"},
{0X0014510F, "SH", "WedgeName"},
{0X00145110, "SH", "WedgeManufacturerName"},
{0X00145111, "LO", "WedgeDescription"},
{0X00145112, "DS", "NominalBeamAngle"},
{0X00145113, "DS", "WedgeOffsetX"},
{0X00145114, "DS", "WedgeOffsetY"},
{0X00145115, "DS", "WedgeTotalLength"},
{0X00145116, "DS", "WedgeInContactLength"},
{0X00145117, "DS", "WedgeFrontGap"},
{0X00145118, "DS", "WedgeTotalHeight"},
{0X00145119, "DS", "WedgeFrontHeight"},
{0X0014511A, "DS", "WedgeRearHeight"},
{0X0014511B, "DS", "WedgeTotalWidth"},
{0X0014511C, "DS", "WedgeInContactWidth"},
{0X0014511D, "DS", "WedgeChamferHeight"},
{0X0014511E, "CS", "WedgeCurve"},
{0X0014511F, "DS", "RadiusAlongWedge"},
{0X00160001, "DS", "WhitePoint"},
{0X00160002, "DS", "PrimaryChromaticities"},
{0X00160003, "UT", "BatteryLevel"},
{0X00160004, "DS", "ExposureTimeInSeconds"},
{0X00160005, "DS", "FNumber"},
{0X00160006, "IS", "OECFRows"},
{0X00160007, "IS", "OECFColumns"},
{0X00160008, "UC", "OECFColumnNames"},
{0X00160009, "DS", "OECFValues"},
{0X0016000A, "IS", "SpatialFrequencyResponseRows"},
{0X0016000B, "IS", "SpatialFrequencyResponseColumns"},
{0X0016000C, "UC", "SpatialFrequencyResponseColumnNames"},
{0X0016000D, "DS", "SpatialFrequencyResponseValues"},
{0X0016000E, "IS", "ColorFilterArrayPatternRows"},
{0X0016000F, "IS", "ColorFilterArrayPatternColumns"},
{0X00160010, "DS", "ColorFilterArrayPatternValues"},
{0X00160011, "US", "FlashFiringStatus"},
{0X00160012, "US", "FlashReturnStatus"},
{0X00160013, "US", "FlashMode"},
{0X00160014, "US", "FlashFunctionPresent"},
{0X00160015, "US", "FlashRedEyeMode"},
{0X00160016, "US", "ExposureProgram"},
{0X00160017, "UT", "SpectralSensitivity"},
{0X00160018, "IS", "PhotographicSensitivity"},
{0X00160019, "IS", "SelfTimerMode"},
{0X0016001A, "US", "SensitivityType"},
{0X0016001B, "IS", "StandardOutputSensitivity"},
{0X0016001C, "IS", "RecommendedExposureIndex"},
{0X0016001D, "IS", "ISOSpeed"},
{0X0016001E, "IS", "ISOSpeedLatitudeyyy"},
{0X0016001F, "IS", "ISOSpeedLatitudezzz"},
{0X00160020, "UT", "EXIFVersion"},
{0X00160021, "DS", "ShutterSpeedValue"},
{0X00160022, "DS", "ApertureValue"},
{0X00160023, "DS", "BrightnessValue"},
{0X00160024, "DS", "ExposureBiasValue"},
{0X00160025, "DS", "MaxApertureValue"},
{0X00160026, "DS", "SubjectDistance"},
{0X00160027, "US", "MeteringMode"},
{0X00160028, "US", "LightSource"},
{0X00160029, "DS", "FocalLength"},
{0X0016002A, "IS", "SubjectArea"},
{0X0016002B, "OB", "MakerNote"},
{0X00160030, "DS", "Temperature"},
{0X00160031, "DS", "Humidity"},
{0X00160032, "DS", "Pressure"},
{0X00160033, "DS", "WaterDepth"},
{0X00160034, "DS", "Acceleration"},
{0X00160035, "DS", "CameraElevationAngle"},
{0X00160036, "DS", "FlashEnergy"},
{0X00160037, "IS", "SubjectLocation"},
{0X00160038, "DS", "PhotographicExposureIndex"},
{0X00160039, "US", "SensingMethod"},
{0X0016003A, "US", "FileSource"},
{0X0016003B, "US", "SceneType"},
{0X00160041, "US", "CustomRendered"},
{0X00160042, "US", "ExposureMode"},
{0X00160043, "US", "WhiteBalance"},
{0X00160044, "DS", "DigitalZoomRatio"},
{0X00160045, "IS", "FocalLengthIn35mmFilm"},
{0X00160046, "US", "SceneCaptureType"},
{0X00160047, "US", "GainControl"},
{0X00160048, "US", "Contrast"},
{0X00160049, "US", "Saturation"},
{0X0016004A, "US", "Sharpness"},
{0X0016004B, "OB", "DeviceSettingDescription"},
{0X0016004C, "US", "SubjectDistanceRange"},
{0X0016004D, "UT", "CameraOwnerName"},
{0X0016004E, "DS", "LensSpecification"},
{0X0016004F, "UT", "LensMake"},
{0X00160050, "UT", "LensModel"},
{0X00160051, "UT", "LensSerialNumber"},
{0X00160061, "CS", "InteroperabilityIndex"},
{0X00160062, "OB", "InteroperabilityVersion"},
{0X00160070, "OB", "GPSVersionID"},
{0X00160071, "CS", "GPSLatitudeRef"},
{0X00160072, "DS", "GPSLatitude"},
{0X00160073, "CS", "GPSLongitudeRef"},
{0X00160074, "DS", "GPSLongitude"},
{0X00160075, "US", "GPSAltitudeRef"},
{0X00160076, "DS", "GPSAltitude"},
{0X00160077, "DT", "GPSTimeStamp"},
{0X00160078, "UT", "GPSSatellites"},
{0X00160079, "CS", "GPSStatus"},
{0X0016007A, "CS", "GPSMeasureMode"},
{0X0016007B, "DS", "GPSDOP"},
{0X0016007C, "CS", "GPSSpeedRef"},
{0X0016007D, "DS", "GPSSpeed"},
{0X0016007E, "CS", "GPSTrackRef"},
{0X0016007F, "DS", "GPSTrack"},
{0X00160080, "CS", "GPSImgDirectionRef"},
{0X00160081, "DS", "GPSImgDirection"},
{0X00160082, "UT", "GPSMapDatum"},
{0X00160083, "CS", "GPSDestLatitudeRef"},
{0X00160084, "DS", "GPSDestLatitude"},
{0X00160085, "CS", "GPSDestLongitudeRef"},
{0X00160086, "DS", "GPSDestLongitude"},
{0X00160087, "CS", "GPSDestBearingRef"},
{0X00160088, "DS", "GPSDestBearing"},
{0X00160089, "CS", "GPSDestDistanceRef"},
{0X0016008A, "DS", "GPSDestDistance"},
{0X0016008B, "OB", "GPSProcessingMethod"},
{0X0016008C, "OB", "GPSAreaInformation"},
{0X0016008D, "DT", "GPSDateStamp"},
{0X0016008E, "IS", "GPSDifferential"},
{0X00161001, "CS", "LightSourcePolarization"},
{0X00161002, "DS", "EmitterColorTemperature"},
{0X00161003, "CS", "ContactMethod"},
{0X00161004, "CS", "ImmersionMedia"},
{0X00161005, "DS", "OpticalMagnificationFactor"},
{0X00180010, "LO", "ContrastBolusAgent"},
{0X00180012, "SQ", "ContrastBolusAgentSequence"},
{0X00180013, "FL", "ContrastBolusT1Relaxivity"},
{0X00180014, "SQ", "ContrastBolusAdministrationRouteSequence"},
{0X00180015, "CS", "BodyPartExamined"},
{0X00180020, "CS", "ScanningSequence"},
{0X00180021, "CS", "SequenceVariant"},
{0X00180022, "CS", "ScanOptions"},
{0X00180023, "CS", "MRAcquisitionType"},
{0X00180024, "SH", "SequenceName"},
{0X00180025, "CS", "AngioFlag"},
{0X00180026, "SQ", "InterventionDrugInformationSequence"},
{0X00180027, "TM", "InterventionDrugStopTime"},
{0X00180028, "DS", "InterventionDrugDose"},
{0X00180029, "SQ", "InterventionDrugCodeSequence"},
{0X0018002A, "SQ", "AdditionalDrugSequence"},
{0X00180030, "LO", "Radionuclide"},
{0X00180031, "LO", "Radiopharmaceutical"},
{0X00180032, "DS", "EnergyWindowCenterline"},
{0X00180033, "DS", "EnergyWindowTotalWidth"},
{0X00180034, "LO", "InterventionDrugName"},
{0X00180035, "TM", "InterventionDrugStartTime"},
{0X00180036, "SQ", "InterventionSequence"},
{0X00180037, "CS", "TherapyType"},
{0X00180038, "CS", "InterventionStatus"},
{0X00180039, "CS", "TherapyDescription"},
{0X0018003A, "ST", "InterventionDescription"},
{0X00180040, "IS", "CineRate"},
{0X00180042, "CS", "InitialCineRunState"},
{0X00180050, "DS", "SliceThickness"},
{0X00180060, "DS", "KVP"},
{0X00180061, "DS", ""},
{0X00180070, "IS", "CountsAccumulated"},
{0X00180071, "CS", "AcquisitionTerminationCondition"},
{0X00180072, "DS", "EffectiveDuration"},
{0X00180073, "CS", "AcquisitionStartCondition"},
{0X00180074, "IS", "AcquisitionStartConditionData"},
{0X00180075, "IS", "AcquisitionTerminationConditionData"},
{0X00180080, "DS", "RepetitionTime"},
{0X00180081, "DS", "EchoTime"},
{0X00180082, "DS", "InversionTime"},
{0X00180083, "DS", "NumberOfAverages"},
{0X00180084, "DS", "ImagingFrequency"},
{0X00180085, "SH", "ImagedNucleus"},
{0X00180086, "IS", "EchoNumbers"},
{0X00180087, "DS", "MagneticFieldStrength"},
{0X00180088, "DS", "SpacingBetweenSlices"},
{0X00180089, "IS", "NumberOfPhaseEncodingSteps"},
{0X00180090, "DS", "DataCollectionDiameter"},
{0X00180091, "IS", "EchoTrainLength"},
{0X00180093, "DS", "PercentSampling"},
{0X00180094, "DS", "PercentPhaseFieldOfView"},
{0X00180095, "DS", "PixelBandwidth"},
{0X00181000, "LO", "DeviceSerialNumber"},
{0X00181002, "UI", "DeviceUID"},
{0X00181003, "LO", "DeviceID"},
{0X00181004, "LO", "PlateID"},
{0X00181005, "LO", "GeneratorID"},
{0X00181006, "LO", "GridID"},
{0X00181007, "LO", "CassetteID"},
{0X00181008, "LO", "GantryID"},
{0X00181009, "UT", "UniqueDeviceIdentifier"},
{0X0018100A, "SQ", "UDISequence"},
{0X0018100B, "UI", "ManufacturerDeviceClassUID"},
{0X00181010, "LO", "SecondaryCaptureDeviceID"},
{0X00181011, "LO", "HardcopyCreationDeviceID"},
{0X00181012, "DA", "DateOfSecondaryCapture"},
{0X00181014, "TM", "TimeOfSecondaryCapture"},
{0X00181016, "LO", "SecondaryCaptureDeviceManufacturer"},
{0X00181017, "LO", "HardcopyDeviceManufacturer"},
{0X00181018, "LO", "SecondaryCaptureDeviceManufacturerModelName"},
{0X00181019, "LO", "SecondaryCaptureDeviceSoftwareVersions"},
{0X0018101A, "LO", "HardcopyDeviceSoftwareVersion"},
{0X0018101B, "LO", "HardcopyDeviceManufacturerModelName"},
{0X00181020, "LO", "SoftwareVersions"},
{0X00181022, "SH", "VideoImageFormatAcquired"},
{0X00181023, "LO", "DigitalImageFormatAcquired"},
{0X00181030, "LO", "ProtocolName"},
{0X00181040, "LO", "ContrastBolusRoute"},
{0X00181041, "DS", "ContrastBolusVolume"},
{0X00181042, "TM", "ContrastBolusStartTime"},
{0X00181043, "TM", "ContrastBolusStopTime"},
{0X00181044, "DS", "ContrastBolusTotalDose"},
{0X00181045, "IS", "SyringeCounts"},
{0X00181046, "DS", "ContrastFlowRate"},
{0X00181047, "DS", "ContrastFlowDuration"},
{0X00181048, "CS", "ContrastBolusIngredient"},
{0X00181049, "DS", "ContrastBolusIngredientConcentration"},
{0X00181050, "DS", "SpatialResolution"},
{0X00181060, "DS", "TriggerTime"},
{0X00181061, "LO", "TriggerSourceOrType"},
{0X00181062, "IS", "NominalInterval"},
{0X00181063, "DS", "FrameTime"},
{0X00181064, "LO", "CardiacFramingType"},
{0X00181065, "DS", "FrameTimeVector"},
{0X00181066, "DS", "FrameDelay"},
{0X00181067, "DS", "ImageTriggerDelay"},
{0X00181068, "DS", "MultiplexGroupTimeOffset"},
{0X00181069, "DS", "TriggerTimeOffset"},
{0X0018106A, "CS", "SynchronizationTrigger"},
{0X0018106C, "US", "SynchronizationChannel"},
{0X0018106E, "UL", "TriggerSamplePosition"},
{0X00181070, "LO", "RadiopharmaceuticalRoute"},
{0X00181071, "DS", "RadiopharmaceuticalVolume"},
{0X00181072, "TM", "RadiopharmaceuticalStartTime"},
{0X00181073, "TM", "RadiopharmaceuticalStopTime"},
{0X00181074, "DS", "RadionuclideTotalDose"},
{0X00181075, "DS", "RadionuclideHalfLife"},
{0X00181076, "DS", "RadionuclidePositronFraction"},
{0X00181077, "DS", "RadiopharmaceuticalSpecificActivity"},
{0X00181078, "DT", "RadiopharmaceuticalStartDateTime"},
{0X00181079, "DT", "RadiopharmaceuticalStopDateTime"},
{0X00181080, "CS", "BeatRejectionFlag"},
{0X00181081, "IS", "LowRRValue"},
{0X00181082, "IS", "HighRRValue"},
{0X00181083, "IS", "IntervalsAcquired"},
{0X00181084, "IS", "IntervalsRejected"},
{0X00181085, "LO", "PVCRejection"},
{0X00181086, "IS", "SkipBeats"},
{0X00181088, "IS", "HeartRate"},
{0X00181090, "IS", "CardiacNumberOfImages"},
{0X00181094, "IS", "TriggerWindow"},
{0X00181100, "DS", "ReconstructionDiameter"},
{0X00181110, "DS", "DistanceSourceToDetector"},
{0X00181111, "DS", "DistanceSourceToPatient"},
{0X00181114, "DS", "EstimatedRadiographicMagnificationFactor"},
{0X00181120, "DS", "GantryDetectorTilt"},
{0X00181121, "DS", "GantryDetectorSlew"},
{0X00181130, "DS", "TableHeight"},
{0X00181131, "DS", "TableTraverse"},
{0X00181134, "CS", "TableMotion"},
{0X00181135, "DS", "TableVerticalIncrement"},
{0X00181136, "DS", "TableLateralIncrement"},
{0X00181137, "DS", "TableLongitudinalIncrement"},
{0X00181138, "DS", "TableAngle"},
{0X0018113A, "CS", "TableType"},
{0X00181140, "CS", "RotationDirection"},
{0X00181141, "DS", "AngularPosition"},
{0X00181142, "DS", "RadialPosition"},
{0X00181143, "DS", "ScanArc"},
{0X00181144, "DS", "AngularStep"},
{0X00181145, "DS", "CenterOfRotationOffset"},
{0X00181146, "DS", "RotationOffset"},
{0X00181147, "CS", "FieldOfViewShape"},
{0X00181149, "IS", "FieldOfViewDimensions"},
{0X00181150, "IS", "ExposureTime"},
{0X00181151, "IS", "XRayTubeCurrent"},
{0X00181152, "IS", "Exposure"},
{0X00181153, "IS", "ExposureInuAs"},
{0X00181154, "DS", "AveragePulseWidth"},
{0X00181155, "CS", "RadiationSetting"},
{0X00181156, "CS", "RectificationType"},
{0X0018115A, "CS", "RadiationMode"},
{0X0018115E, "DS", "ImageAndFluoroscopyAreaDoseProduct"},
{0X00181160, "SH", "FilterType"},
{0X00181161, "LO", "TypeOfFilters"},
{0X00181162, "DS", "IntensifierSize"},
{0X00181164, "DS", "ImagerPixelSpacing"},
{0X00181166, "CS", "Grid"},
{0X00181170, "IS", "GeneratorPower"},
{0X00181180, "SH", "CollimatorGridName"},
{0X00181181, "CS", "CollimatorType"},
{0X00181182, "IS", "FocalDistance"},
{0X00181183, "DS", "XFocusCenter"},
{0X00181184, "DS", "YFocusCenter"},
{0X00181190, "DS", "FocalSpots"},
{0X00181191, "CS", "AnodeTargetMaterial"},
{0X001811A0, "DS", "BodyPartThickness"},
{0X001811A2, "DS", "CompressionForce"},
{0X001811A3, "DS", "CompressionPressure"},
{0X001811A4, "LO", "PaddleDescription"},
{0X001811A5, "DS", "CompressionContactArea"},
{0X001811B0, "LO", "AcquisitionMode"},
{0X001811B1, "LO", "DoseModeName"},
{0X001811B2, "CS", "AcquiredSubtractionMaskFlag"},
{0X001811B3, "CS", "FluoroscopyPersistenceFlag"},
{0X001811B4, "CS", "FluoroscopyLastImageHoldPersistenceFlag"},
{0X001811B5, "IS", "UpperLimitNumberOfPersistentFluoroscopyFrames"},
{0X001811B6, "CS", "ContrastBolusAutoInjectionTriggerFlag"},
{0X001811B7, "FD", "ContrastBolusInjectionDelay"},
{0X001811B8, "SQ", "XAAcquisitionPhaseDetailsSequence"},
{0X001811B9, "FD", "XAAcquisitionFrameRate"},
{0X001811BA, "SQ", "XAPlaneDetailsSequence"},
{0X001811BB, "LO", "AcquisitionFieldOfViewLabel"},
{0X001811BC, "SQ", "XRayFilterDetailsSequence"},
{0X001811BD, "FD", "XAAcquisitionDuration"},
{0X001811BE, "CS", "ReconstructionPipelineType"},
{0X001811BF, "SQ", "ImageFilterDetailsSequence"},
{0X001811C0, "CS", "AppliedMaskSubtractionFlag"},
{0X001811C1, "SQ", "RequestedSeriesDescriptionCodeSequence"},
{0X00181200, "DA", "DateOfLastCalibration"},
{0X00181201, "TM", "TimeOfLastCalibration"},
{0X00181202, "DT", "DateTimeOfLastCalibration"},
{0X00181210, "SH", "ConvolutionKernel"},
{0X00181240, "IS", "UpperLowerPixelValues"},
{0X00181242, "IS", "ActualFrameDuration"},
{0X00181243, "IS", "CountRate"},
{0X00181244, "US", "PreferredPlaybackSequencing"},
{0X00181250, "SH", "ReceiveCoilName"},
{0X00181251, "SH", "TransmitCoilName"},
{0X00181260, "SH", "PlateType"},
{0X00181261, "LO", "PhosphorType"},
{0X00181271, "FD", "WaterEquivalentDiameter"},
{0X00181272, "SQ", "WaterEquivalentDiameterCalculationMethodCodeSequence"},
{0X00181300, "DS", "ScanVelocity"},
{0X00181301, "CS", "WholeBodyTechnique"},
{0X00181302, "IS", "ScanLength"},
{0X00181310, "US", "AcquisitionMatrix"},
{0X00181312, "CS", "InPlanePhaseEncodingDirection"},
{0X00181314, "DS", "FlipAngle"},
{0X00181315, "CS", "VariableFlipAngleFlag"},
{0X00181316, "DS", "SAR"},
{0X00181318, "DS", "dBdt"},
{0X00181320, "FL", "B1rms"},
{0X00181400, "LO", "AcquisitionDeviceProcessingDescription"},
{0X00181401, "LO", "AcquisitionDeviceProcessingCode"},
{0X00181402, "CS", "CassetteOrientation"},
{0X00181403, "CS", "CassetteSize"},
{0X00181404, "US", "ExposuresOnPlate"},
{0X00181405, "IS", "RelativeXRayExposure"},
{0X00181411, "DS", "ExposureIndex"},
{0X00181412, "DS", "TargetExposureIndex"},
{0X00181413, "DS", "DeviationIndex"},
{0X00181450, "DS", "ColumnAngulation"},
{0X00181460, "DS", "TomoLayerHeight"},
{0X00181470, "DS", "TomoAngle"},
{0X00181480, "DS", "TomoTime"},
{0X00181490, "CS", "TomoType"},
{0X00181491, "CS", "TomoClass"},
{0X00181495, "IS", "NumberOfTomosynthesisSourceImages"},
{0X00181500, "CS", "PositionerMotion"},
{0X00181508, "CS", "PositionerType"},
{0X00181510, "DS", "PositionerPrimaryAngle"},
{0X00181511, "DS", "PositionerSecondaryAngle"},
{0X00181520, "DS", "PositionerPrimaryAngleIncrement"},
{0X00181521, "DS", "PositionerSecondaryAngleIncrement"},
{0X00181530, "DS", "DetectorPrimaryAngle"},
{0X00181531, "DS", "DetectorSecondaryAngle"},
{0X00181600, "CS", "ShutterShape"},
{0X00181602, "IS", "ShutterLeftVerticalEdge"},
{0X00181604, "IS", "ShutterRightVerticalEdge"},
{0X00181606, "IS", "ShutterUpperHorizontalEdge"},
{0X00181608, "IS", "ShutterLowerHorizontalEdge"},
{0X00181610, "IS", "CenterOfCircularShutter"},
{0X00181612, "IS", "RadiusOfCircularShutter"},
{0X00181620, "IS", "VerticesOfThePolygonalShutter"},
{0X00181622, "US", "ShutterPresentationValue"},
{0X00181623, "US", "ShutterOverlayGroup"},
{0X00181624, "US", "ShutterPresentationColorCIELabValue"},
{0X00181630, "CS", "OutlineShapeType"},
{0X00181631, "FD", "OutlineLeftVerticalEdge"},
{0X00181632, "FD", "OutlineRightVerticalEdge"},
{0X00181633, "FD", "OutlineUpperHorizontalEdge"},
{0X00181634, "FD", "OutlineLowerHorizontalEdge"},
{0X00181635, "FD", "CenterOfCircularOutline"},
{0X00181636, "FD", "DiameterOfCircularOutline"},
{0X00181637, "UL", "NumberOfPolygonalVertices"},
{0X00181638, "OF", "VerticesOfThePolygonalOutline"},
{0X00181700, "CS", "CollimatorShape"},
{0X00181702, "IS", "CollimatorLeftVerticalEdge"},
{0X00181704, "IS", "CollimatorRightVerticalEdge"},
{0X00181706, "IS", "CollimatorUpperHorizontalEdge"},
{0X00181708, "IS", "CollimatorLowerHorizontalEdge"},
{0X00181710, "IS", "CenterOfCircularCollimator"},
{0X00181712, "IS", "RadiusOfCircularCollimator"},
{0X00181720, "IS", "VerticesOfThePolygonalCollimator"},
{0X00181800, "CS", "AcquisitionTimeSynchronized"},
{0X00181801, "SH", "TimeSource"},
{0X00181802, "CS", "TimeDistributionProtocol"},
{0X00181803, "LO", "NTPSourceAddress"},
{0X00182001, "IS", "PageNumberVector"},
{0X00182002, "SH", "FrameLabelVector"},
{0X00182003, "DS", "FramePrimaryAngleVector"},
{0X00182004, "DS", "FrameSecondaryAngleVector"},
{0X00182005, "DS", "SliceLocationVector"},
{0X00182006, "SH", "DisplayWindowLabelVector"},
{0X00182010, "DS", "NominalScannedPixelSpacing"},
{0X00182020, "CS", "DigitizingDeviceTransportDirection"},
{0X00182030, "DS", "RotationOfScannedFilm"},
{0X00182041, "SQ", "BiopsyTargetSequence"},
{0X00182042, "UI", "TargetUID"},
{0X00182043, "FL", "LocalizingCursorPosition"},
{0X00182044, "FL", "CalculatedTargetPosition"},
{0X00182045, "SH", "TargetLabel"},
{0X00182046, "FL", "DisplayedZValue"},
{0X00183100, "CS", "IVUSAcquisition"},
{0X00183101, "DS", "IVUSPullbackRate"},
{0X00183102, "DS", "IVUSGatedRate"},
{0X00183103, "IS", "IVUSPullbackStartFrameNumber"},
{0X00183104, "IS", "IVUSPullbackStopFrameNumber"},
{0X00183105, "IS", "LesionNumber"},
{0X00184000, "LT", "AcquisitionComments"},
{0X00185000, "SH", "OutputPower"},
{0X00185010, "LO", "TransducerData"},
{0X00185011, "SQ", "TransducerIdentificationSequence"},
{0X00185012, "DS", "FocusDepth"},
{0X00185020, "LO", "ProcessingFunction"},
{0X00185021, "LO", "PostprocessingFunction"},
{0X00185022, "DS", "MechanicalIndex"},
{0X00185024, "DS", "BoneThermalIndex"},
{0X00185026, "DS", "CranialThermalIndex"},
{0X00185027, "DS", "SoftTissueThermalIndex"},
{0X00185028, "DS", "SoftTissueFocusThermalIndex"},
{0X00185029, "DS", "SoftTissueSurfaceThermalIndex"},
{0X00185030, "DS", "DynamicRange"},
{0X00185040, "DS", "TotalGain"},
{0X00185050, "IS", "DepthOfScanField"},
{0X00185100, "CS", "PatientPosition"},
{0X00185101, "CS", "ViewPosition"},
{0X00185104, "SQ", "ProjectionEponymousNameCodeSequence"},
{0X00185210, "DS", "ImageTransformationMatrix"},
{0X00185212, "DS", "ImageTranslationVector"},
{0X00186000, "DS", "Sensitivity"},
{0X00186011, "SQ", "SequenceOfUltrasoundRegions"},
{0X00186012, "US", "RegionSpatialFormat"},
{0X00186014, "US", "RegionDataType"},
{0X00186016, "UL", "RegionFlags"},
{0X00186018, "UL", "RegionLocationMinX0"},
{0X0018601A, "UL", "RegionLocationMinY0"},
{0X0018601C, "UL", "RegionLocationMaxX1"},
{0X0018601E, "UL", "RegionLocationMaxY1"},
{0X00186020, "SL", "ReferencePixelX0"},
{0X00186022, "SL", "ReferencePixelY0"},
{0X00186024, "US", "PhysicalUnitsXDirection"},
{0X00186026, "US", "PhysicalUnitsYDirection"},
{0X00186028, "FD", "ReferencePixelPhysicalValueX"},
{0X0018602A, "FD", "ReferencePixelPhysicalValueY"},
{0X0018602C, "FD", "PhysicalDeltaX"},
{0X0018602E, "FD", "PhysicalDeltaY"},
{0X00186030, "UL", "TransducerFrequency"},
{0X00186031, "CS", "TransducerType"},
{0X00186032, "UL", "PulseRepetitionFrequency"},
{0X00186034, "FD", "DopplerCorrectionAngle"},
{0X00186036, "FD", "SteeringAngle"},
{0X00186038, "UL", "DopplerSampleVolumeXPositionRetired"},
{0X00186039, "SL", "DopplerSampleVolumeXPosition"},
{0X0018603A, "UL", "DopplerSampleVolumeYPositionRetired"},
{0X0018603B, "SL", "DopplerSampleVolumeYPosition"},
{0X0018603C, "UL", "TMLinePositionX0Retired"},
{0X0018603D, "SL", "TMLinePositionX0"},
{0X0018603E, "UL", "TMLinePositionY0Retired"},
{0X0018603F, "SL", "TMLinePositionY0"},
{0X00186040, "UL", "TMLinePositionX1Retired"},
{0X00186041, "SL", "TMLinePositionX1"},
{0X00186042, "UL", "TMLinePositionY1Retired"},
{0X00186043, "SL", "TMLinePositionY1"},
{0X00186044, "US", "PixelComponentOrganization"},
{0X00186046, "UL", "PixelComponentMask"},
{0X00186048, "UL", "PixelComponentRangeStart"},
{0X0018604A, "UL", "PixelComponentRangeStop"},
{0X0018604C, "US", "PixelComponentPhysicalUnits"},
{0X0018604E, "US", "PixelComponentDataType"},
{0X00186050, "UL", "NumberOfTableBreakPoints"},
{0X00186052, "UL", "TableOfXBreakPoints"},
{0X00186054, "FD", "TableOfYBreakPoints"},
{0X00186056, "UL", "NumberOfTableEntries"},
{0X00186058, "UL", "TableOfPixelValues"},
{0X0018605A, "FL", "TableOfParameterValues"},
{0X00186060, "FL", "RWaveTimeVector"},
{0X00186070, "US", "ActiveImageAreaOverlayGroup"},
{0X00187000, "CS", "DetectorConditionsNominalFlag"},
{0X00187001, "DS", "DetectorTemperature"},
{0X00187004, "CS", "DetectorType"},
{0X00187005, "CS", "DetectorConfiguration"},
{0X00187006, "LT", "DetectorDescription"},
{0X00187008, "LT", "DetectorMode"},
{0X0018700A, "SH", "DetectorID"},
{0X0018700C, "DA", "DateOfLastDetectorCalibration"},
{0X0018700E, "TM", "TimeOfLastDetectorCalibration"},
{0X00187010, "IS", "ExposuresOnDetectorSinceLastCalibration"},
{0X00187011, "IS", "ExposuresOnDetectorSinceManufactured"},
{0X00187012, "DS", "DetectorTimeSinceLastExposure"},
{0X00187014, "DS", "DetectorActiveTime"},
{0X00187016, "DS", "DetectorActivationOffsetFromExposure"},
{0X0018701A, "DS", "DetectorBinning"},
{0X00187020, "DS", "DetectorElementPhysicalSize"},
{0X00187022, "DS", "DetectorElementSpacing"},
{0X00187024, "CS", "DetectorActiveShape"},
{0X00187026, "DS", "DetectorActiveDimensions"},
{0X00187028, "DS", "DetectorActiveOrigin"},
{0X0018702A, "LO", "DetectorManufacturerName"},
{0X0018702B, "LO", "DetectorManufacturerModelName"},
{0X00187030, "DS", "FieldOfViewOrigin"},
{0X00187032, "DS", "FieldOfViewRotation"},
{0X00187034, "CS", "FieldOfViewHorizontalFlip"},
{0X00187036, "FL", "PixelDataAreaOriginRelativeToFOV"},
{0X00187038, "FL", "PixelDataAreaRotationAngleRelativeToFOV"},
{0X00187040, "LT", "GridAbsorbingMaterial"},
{0X00187041, "LT", "GridSpacingMaterial"},
{0X00187042, "DS", "GridThickness"},
{0X00187044, "DS", "GridPitch"},
{0X00187046, "IS", "GridAspectRatio"},
{0X00187048, "DS", "GridPeriod"},
{0X0018704C, "DS", "GridFocalDistance"},
{0X00187050, "CS", "FilterMaterial"},
{0X00187052, "DS", "FilterThicknessMinimum"},
{0X00187054, "DS", "FilterThicknessMaximum"},
{0X00187056, "FL", "FilterBeamPathLengthMinimum"},
{0X00187058, "FL", "FilterBeamPathLengthMaximum"},
{0X00187060, "CS", "ExposureControlMode"},
{0X00187062, "LT", "ExposureControlModeDescription"},
{0X00187064, "CS", "ExposureStatus"},
{0X00187065, "DS", "PhototimerSetting"},
{0X00188150, "DS", "ExposureTimeInuS"},
{0X00188151, "DS", "XRayTubeCurrentInuA"},
{0X00189004, "CS", "ContentQualification"},
{0X00189005, "SH", "PulseSequenceName"},
{0X00189006, "SQ", "MRImagingModifierSequence"},
{0X00189008, "CS", "EchoPulseSequence"},
{0X00189009, "CS", "InversionRecovery"},
{0X00189010, "CS", "FlowCompensation"},
{0X00189011, "CS", "MultipleSpinEcho"},
{0X00189012, "CS", "MultiPlanarExcitation"},
{0X00189014, "CS", "PhaseContrast"},
{0X00189015, "CS", "TimeOfFlightContrast"},
{0X00189016, "CS", "Spoiling"},
{0X00189017, "CS", "SteadyStatePulseSequence"},
{0X00189018, "CS", "EchoPlanarPulseSequence"},
{0X00189019, "FD", "TagAngleFirstAxis"},
{0X00189020, "CS", "MagnetizationTransfer"},
{0X00189021, "CS", "T2Preparation"},
{0X00189022, "CS", "BloodSignalNulling"},
{0X00189024, "CS", "SaturationRecovery"},
{0X00189025, "CS", "SpectrallySelectedSuppression"},
{0X00189026, "CS", "SpectrallySelectedExcitation"},
{0X00189027, "CS", "SpatialPresaturation"},
{0X00189028, "CS", "Tagging"},
{0X00189029, "CS", "OversamplingPhase"},
{0X00189030, "FD", "TagSpacingFirstDimension"},
{0X00189032, "CS", "GeometryOfKSpaceTraversal"},
{0X00189033, "CS", "SegmentedKSpaceTraversal"},
{0X00189034, "CS", "RectilinearPhaseEncodeReordering"},
{0X00189035, "FD", "TagThickness"},
{0X00189036, "CS", "PartialFourierDirection"},
{0X00189037, "CS", "CardiacSynchronizationTechnique"},
{0X00189041, "LO", "ReceiveCoilManufacturerName"},
{0X00189042, "SQ", "MRReceiveCoilSequence"},
{0X00189043, "CS", "ReceiveCoilType"},
{0X00189044, "CS", "QuadratureReceiveCoil"},
{0X00189045, "SQ", "MultiCoilDefinitionSequence"},
{0X00189046, "LO", "MultiCoilConfiguration"},
{0X00189047, "SH", "MultiCoilElementName"},
{0X00189048, "CS", "MultiCoilElementUsed"},
{0X00189049, "SQ", "MRTransmitCoilSequence"},
{0X00189050, "LO", "TransmitCoilManufacturerName"},
{0X00189051, "CS", "TransmitCoilType"},
{0X00189052, "FD", "SpectralWidth"},
{0X00189053, "FD", "ChemicalShiftReference"},
{0X00189054, "CS", "VolumeLocalizationTechnique"},
{0X00189058, "US", "MRAcquisitionFrequencyEncodingSteps"},
{0X00189059, "CS", "Decoupling"},
{0X00189060, "CS", "DecoupledNucleus"},
{0X00189061, "FD", "DecouplingFrequency"},
{0X00189062, "CS", "DecouplingMethod"},
{0X00189063, "FD", "DecouplingChemicalShiftReference"},
{0X00189064, "CS", "KSpaceFiltering"},
{0X00189065, "CS", "TimeDomainFiltering"},
{0X00189066, "US", "NumberOfZeroFills"},
{0X00189067, "CS", "BaselineCorrection"},
{0X00189069, "FD", "ParallelReductionFactorInPlane"},
{0X00189070, "FD", "CardiacRRIntervalSpecified"},
{0X00189073, "FD", "AcquisitionDuration"},
{0X00189074, "DT", "FrameAcquisitionDateTime"},
{0X00189075, "CS", "DiffusionDirectionality"},
{0X00189076, "SQ", "DiffusionGradientDirectionSequence"},
{0X00189077, "CS", "ParallelAcquisition"},
{0X00189078, "CS", "ParallelAcquisitionTechnique"},
{0X00189079, "FD", "InversionTimes"},
{0X00189080, "ST", "MetaboliteMapDescription"},
{0X00189081, "CS", "PartialFourier"},
{0X00189082, "FD", "EffectiveEchoTime"},
{0X00189083, "SQ", "MetaboliteMapCodeSequence"},
{0X00189084, "SQ", "ChemicalShiftSequence"},
{0X00189085, "CS", "CardiacSignalSource"},
{0X00189087, "FD", "DiffusionBValue"},
{0X00189089, "FD", "DiffusionGradientOrientation"},
{0X00189090, "FD", "VelocityEncodingDirection"},
{0X00189091, "FD", "VelocityEncodingMinimumValue"},
{0X00189092, "SQ", "VelocityEncodingAcquisitionSequence"},
{0X00189093, "US", "NumberOfKSpaceTrajectories"},
{0X00189094, "CS", "CoverageOfKSpace"},
{0X00189095, "UL", "SpectroscopyAcquisitionPhaseRows"},
{0X00189096, "FD", "ParallelReductionFactorInPlaneRetired"},
{0X00189098, "FD", "TransmitterFrequency"},
{0X00189100, "CS", "ResonantNucleus"},
{0X00189101, "CS", "FrequencyCorrection"},
{0X00189103, "SQ", "MRSpectroscopyFOVGeometrySequence"},
{0X00189104, "FD", "SlabThickness"},
{0X00189105, "FD", "SlabOrientation"},
{0X00189106, "FD", "MidSlabPosition"},
{0X00189107, "SQ", "MRSpatialSaturationSequence"},
{0X00189112, "SQ", "MRTimingAndRelatedParametersSequence"},
{0X00189114, "SQ", "MREchoSequence"},
{0X00189115, "SQ", "MRModifierSequence"},
{0X00189117, "SQ", "MRDiffusionSequence"},
{0X00189118, "SQ", "CardiacSynchronizationSequence"},
{0X00189119, "SQ", "MRAveragesSequence"},
{0X00189125, "SQ", "MRFOVGeometrySequence"},
{0X00189126, "SQ", "VolumeLocalizationSequence"},
{0X00189127, "UL", "SpectroscopyAcquisitionDataColumns"},
{0X00189147, "CS", "DiffusionAnisotropyType"},
{0X00189151, "DT", "FrameReferenceDateTime"},
{0X00189152, "SQ", "MRMetaboliteMapSequence"},
{0X00189155, "FD", "ParallelReductionFactorOutOfPlane"},
{0X00189159, "UL", "SpectroscopyAcquisitionOutOfPlanePhaseSteps"},
{0X00189166, "CS", "BulkMotionStatus"},
{0X00189168, "FD", "ParallelReductionFactorSecondInPlane"},
{0X00189169, "CS", "CardiacBeatRejectionTechnique"},
{0X00189170, "CS", "RespiratoryMotionCompensationTechnique"},
{0X00189171, "CS", "RespiratorySignalSource"},
{0X00189172, "CS", "BulkMotionCompensationTechnique"},
{0X00189173, "CS", "BulkMotionSignalSource"},
{0X00189174, "CS", "ApplicableSafetyStandardAgency"},
{0X00189175, "LO", "ApplicableSafetyStandardDescription"},
{0X00189176, "SQ", "OperatingModeSequence"},
{0X00189177, "CS", "OperatingModeType"},
{0X00189178, "CS", "OperatingMode"},
{0X00189179, "CS", "SpecificAbsorptionRateDefinition"},
{0X00189180, "CS", "GradientOutputType"},
{0X00189181, "FD", "SpecificAbsorptionRateValue"},
{0X00189182, "FD", "GradientOutput"},
{0X00189183, "CS", "FlowCompensationDirection"},
{0X00189184, "FD", "TaggingDelay"},
{0X00189185, "ST", "RespiratoryMotionCompensationTechniqueDescription"},
{0X00189186, "SH", "RespiratorySignalSourceID"},
{0X00189195, "FD", "ChemicalShiftMinimumIntegrationLimitInHz"},
{0X00189196, "FD", "ChemicalShiftMaximumIntegrationLimitInHz"},
{0X00189197, "SQ", "MRVelocityEncodingSequence"},
{0X00189198, "CS", "FirstOrderPhaseCorrection"},
{0X00189199, "CS", "WaterReferencedPhaseCorrection"},
{0X00189200, "CS", "MRSpectroscopyAcquisitionType"},
{0X00189214, "CS", "RespiratoryCyclePosition"},
{0X00189217, "FD", "VelocityEncodingMaximumValue"},
{0X00189218, "FD", "TagSpacingSecondDimension"},
{0X00189219, "SS", "TagAngleSecondAxis"},
{0X00189220, "FD", "FrameAcquisitionDuration"},
{0X00189226, "SQ", "MRImageFrameTypeSequence"},
{0X00189227, "SQ", "MRSpectroscopyFrameTypeSequence"},
{0X00189231, "US", "MRAcquisitionPhaseEncodingStepsInPlane"},
{0X00189232, "US", "MRAcquisitionPhaseEncodingStepsOutOfPlane"},
{0X00189234, "UL", "SpectroscopyAcquisitionPhaseColumns"},
{0X00189236, "CS", "CardiacCyclePosition"},
{0X00189239, "SQ", "SpecificAbsorptionRateSequence"},
{0X00189240, "US", "RFEchoTrainLength"},
{0X00189241, "US", "GradientEchoTrainLength"},
{0X00189250, "CS", "ArterialSpinLabelingContrast"},
{0X00189251, "SQ", "MRArterialSpinLabelingSequence"},
{0X00189252, "LO", "ASLTechniqueDescription"},
{0X00189253, "US", "ASLSlabNumber"},
{0X00189254, "FD", "ASLSlabThickness"},
{0X00189255, "FD", "ASLSlabOrientation"},
{0X00189256, "FD", "ASLMidSlabPosition"},
{0X00189257, "CS", "ASLContext"},
{0X00189258, "UL", "ASLPulseTrainDuration"},
{0X00189259, "CS", "ASLCrusherFlag"},
{0X0018925A, "FD", "ASLCrusherFlowLimit"},
{0X0018925B, "LO", "ASLCrusherDescription"},
{0X0018925C, "CS", "ASLBolusCutoffFlag"},
{0X0018925D, "SQ", "ASLBolusCutoffTimingSequence"},
{0X0018925E, "LO", "ASLBolusCutoffTechnique"},
{0X0018925F, "UL", "ASLBolusCutoffDelayTime"},
{0X00189260, "SQ", "ASLSlabSequence"},
{0X00189295, "FD", "ChemicalShiftMinimumIntegrationLimitInppm"},
{0X00189296, "FD", "ChemicalShiftMaximumIntegrationLimitInppm"},
{0X00189297, "CS", "WaterReferenceAcquisition"},
{0X00189298, "IS", "EchoPeakPosition"},
{0X00189301, "SQ", "CTAcquisitionTypeSequence"},
{0X00189302, "CS", "AcquisitionType"},
{0X00189303, "FD", "TubeAngle"},
{0X00189304, "SQ", "CTAcquisitionDetailsSequence"},
{0X00189305, "FD", "RevolutionTime"},
{0X00189306, "FD", "SingleCollimationWidth"},
{0X00189307, "FD", "TotalCollimationWidth"},
{0X00189308, "SQ", "CTTableDynamicsSequence"},
{0X00189309, "FD", "TableSpeed"},
{0X00189310, "FD", "TableFeedPerRotation"},
{0X00189311, "FD", "SpiralPitchFactor"},
{0X00189312, "SQ", "CTGeometrySequence"},
{0X00189313, "FD", "DataCollectionCenterPatient"},
{0X00189314, "SQ", "CTReconstructionSequence"},
{0X00189315, "CS", "ReconstructionAlgorithm"},
{0X00189316, "CS", "ConvolutionKernelGroup"},
{0X00189317, "FD", "ReconstructionFieldOfView"},
{0X00189318, "FD", "ReconstructionTargetCenterPatient"},
{0X00189319, "FD", "ReconstructionAngle"},
{0X00189320, "SH", "ImageFilter"},
{0X00189321, "SQ", "CTExposureSequence"},
{0X00189322, "FD", "ReconstructionPixelSpacing"},
{0X00189323, "CS", "ExposureModulationType"},
{0X00189324, "FD", "EstimatedDoseSaving"},
{0X00189325, "SQ", "CTXRayDetailsSequence"},
{0X00189326, "SQ", "CTPositionSequence"},
{0X00189327, "FD", "TablePosition"},
{0X00189328, "FD", "ExposureTimeInms"},
{0X00189329, "SQ", "CTImageFrameTypeSequence"},
{0X00189330, "FD", "XRayTubeCurrentInmA"},
{0X00189332, "FD", "ExposureInmAs"},
{0X00189333, "CS", "ConstantVolumeFlag"},
{0X00189334, "CS", "FluoroscopyFlag"},
{0X00189335, "FD", "DistanceSourceToDataCollectionCenter"},
{0X00189337, "US", "ContrastBolusAgentNumber"},
{0X00189338, "SQ", "ContrastBolusIngredientCodeSequence"},
{0X00189340, "SQ", "ContrastAdministrationProfileSequence"},
{0X00189341, "SQ", "ContrastBolusUsageSequence"},
{0X00189342, "CS", "ContrastBolusAgentAdministered"},
{0X00189343, "CS", "ContrastBolusAgentDetected"},
{0X00189344, "CS", "ContrastBolusAgentPhase"},
{0X00189345, "FD", "CTDIvol"},
{0X00189346, "SQ", "CTDIPhantomTypeCodeSequence"},
{0X00189351, "FL", "CalciumScoringMassFactorPatient"},
{0X00189352, "FL", "CalciumScoringMassFactorDevice"},
{0X00189353, "FL", "EnergyWeightingFactor"},
{0X00189360, "SQ", "CTAdditionalXRaySourceSequence"},
{0X00189361, "CS", "MultienergyCTAcquisition"},
{0X00189362, "SQ", "MultienergyCTAcquisitionSequence"},
{0X00189363, "SQ", "MultienergyCTProcessingSequence"},
{0X00189364, "SQ", "MultienergyCTCharacteristicsSequence"},
{0X00189365, "SQ", "MultienergyCTXRaySourceSequence"},
{0X00189366, "US", "XRaySourceIndex"},
{0X00189367, "UC", "XRaySourceID"},
{0X00189368, "CS", "MultienergySourceTechnique"},
{0X00189369, "DT", "SourceStartDateTime"},
{0X0018936A, "DT", "SourceEndDateTime"},
{0X0018936B, "US", "SwitchingPhaseNumber"},
{0X0018936C, "DS", "SwitchingPhaseNominalDuration"},
{0X0018936D, "DS", "SwitchingPhaseTransitionDuration"},
{0X0018936E, "DS", "EffectiveBinEnergy"},
{0X0018936F, "SQ", "MultienergyCTXRayDetectorSequence"},
{0X00189370, "US", "XRayDetectorIndex"},
{0X00189371, "UC", "XRayDetectorID"},
{0X00189372, "CS", "MultienergyDetectorType"},
{0X00189373, "ST", "XRayDetectorLabel"},
{0X00189374, "DS", "NominalMaxEnergy"},
{0X00189375, "DS", "NominalMinEnergy"},
{0X00189376, "US", "ReferencedXRayDetectorIndex"},
{0X00189377, "US", "ReferencedXRaySourceIndex"},
{0X00189378, "US", "ReferencedPathIndex"},
{0X00189379, "SQ", "MultienergyCTPathSequence"},
{0X0018937A, "US", "MultienergyCTPathIndex"},
{0X0018937B, "UT", "MultienergyAcquisitionDescription"},
{0X0018937C, "FD", "MonoenergeticEnergyEquivalent"},
{0X0018937D, "SQ", "MaterialCodeSequence"},
{0X0018937E, "CS", "DecompositionMethod"},
{0X0018937F, "UT", "DecompositionDescription"},
{0X00189380, "SQ", "DecompositionAlgorithmIdentificationSequence"},
{0X00189381, "SQ", "DecompositionMaterialSequence"},
{0X00189382, "SQ", "MaterialAttenuationSequence"},
{0X00189383, "DS", "PhotonEnergy"},
{0X00189384, "DS", "XRayMassAttenuationCoefficient"},
{0X00189401, "SQ", "ProjectionPixelCalibrationSequence"},
{0X00189402, "FL", "DistanceSourceToIsocenter"},
{0X00189403, "FL", "DistanceObjectToTableTop"},
{0X00189404, "FL", "ObjectPixelSpacingInCenterOfBeam"},
{0X00189405, "SQ", "PositionerPositionSequence"},
{0X00189406, "SQ", "TablePositionSequence"},
{0X00189407, "SQ", "CollimatorShapeSequence"},
{0X00189410, "CS", "PlanesInAcquisition"},
{0X00189412, "SQ", "XAXRFFrameCharacteristicsSequence"},
{0X00189417, "SQ", "FrameAcquisitionSequence"},
{0X00189420, "CS", "XRayReceptorType"},
{0X00189423, "LO", "AcquisitionProtocolName"},
{0X00189424, "LT", "AcquisitionProtocolDescription"},
{0X00189425, "CS", "ContrastBolusIngredientOpaque"},
{0X00189426, "FL", "DistanceReceptorPlaneToDetectorHousing"},
{0X00189427, "CS", "IntensifierActiveShape"},
{0X00189428, "FL", "IntensifierActiveDimensions"},
{0X00189429, "FL", "PhysicalDetectorSize"},
{0X00189430, "FL", "PositionOfIsocenterProjection"},
{0X00189432, "SQ", "FieldOfViewSequence"},
{0X00189433, "LO", "FieldOfViewDescription"},
{0X00189434, "SQ", "ExposureControlSensingRegionsSequence"},
{0X00189435, "CS", "ExposureControlSensingRegionShape"},
{0X00189436, "SS", "ExposureControlSensingRegionLeftVerticalEdge"},
{0X00189437, "SS", "ExposureControlSensingRegionRightVerticalEdge"},
{0X00189438, "SS", "ExposureControlSensingRegionUpperHorizontalEdge"},
{0X00189439, "SS", "ExposureControlSensingRegionLowerHorizontalEdge"},
{0X00189440, "SS", "CenterOfCircularExposureControlSensingRegion"},
{0X00189441, "US", "RadiusOfCircularExposureControlSensingRegion"},
{0X00189442, "SS", "VerticesOfThePolygonalExposureControlSensingRegion"},
{0X00189445, "OB", ""},
{0X00189447, "FL", "ColumnAngulationPatient"},
{0X00189449, "FL", "BeamAngle"},
{0X00189451, "SQ", "FrameDetectorParametersSequence"},
{0X00189452, "FL", "CalculatedAnatomyThickness"},
{0X00189455, "SQ", "CalibrationSequence"},
{0X00189456, "SQ", "ObjectThicknessSequence"},
{0X00189457, "CS", "PlaneIdentification"},
{0X00189461, "FL", "FieldOfViewDimensionsInFloat"},
{0X00189462, "SQ", "IsocenterReferenceSystemSequence"},
{0X00189463, "FL", "PositionerIsocenterPrimaryAngle"},
{0X00189464, "FL", "PositionerIsocenterSecondaryAngle"},
{0X00189465, "FL", "PositionerIsocenterDetectorRotationAngle"},
{0X00189466, "FL", "TableXPositionToIsocenter"},
{0X00189467, "FL", "TableYPositionToIsocenter"},
{0X00189468, "FL", "TableZPositionToIsocenter"},
{0X00189469, "FL", "TableHorizontalRotationAngle"},
{0X00189470, "FL", "TableHeadTiltAngle"},
{0X00189471, "FL", "TableCradleTiltAngle"},
{0X00189472, "SQ", "FrameDisplayShutterSequence"},
{0X00189473, "FL", "AcquiredImageAreaDoseProduct"},
{0X00189474, "CS", "CArmPositionerTabletopRelationship"},
{0X00189476, "SQ", "XRayGeometrySequence"},
{0X00189477, "SQ", "IrradiationEventIdentificationSequence"},
{0X00189504, "SQ", "XRay3DFrameTypeSequence"},
{0X00189506, "SQ", "ContributingSourcesSequence"},
{0X00189507, "SQ", "XRay3DAcquisitionSequence"},
{0X00189508, "FL", "PrimaryPositionerScanArc"},
{0X00189509, "FL", "SecondaryPositionerScanArc"},
{0X00189510, "FL", "PrimaryPositionerScanStartAngle"},
{0X00189511, "FL", "SecondaryPositionerScanStartAngle"},
{0X00189514, "FL", "PrimaryPositionerIncrement"},
{0X00189515, "FL", "SecondaryPositionerIncrement"},
{0X00189516, "DT", "StartAcquisitionDateTime"},
{0X00189517, "DT", "EndAcquisitionDateTime"},
{0X00189518, "SS", "PrimaryPositionerIncrementSign"},
{0X00189519, "SS", "SecondaryPositionerIncrementSign"},
{0X00189524, "LO", "ApplicationName"},
{0X00189525, "LO", "ApplicationVersion"},
{0X00189526, "LO", "ApplicationManufacturer"},
{0X00189527, "CS", "AlgorithmType"},
{0X00189528, "LO", "AlgorithmDescription"},
{0X00189530, "SQ", "XRay3DReconstructionSequence"},
{0X00189531, "LO", "ReconstructionDescription"},
{0X00189538, "SQ", "PerProjectionAcquisitionSequence"},
{0X00189541, "SQ", "DetectorPositionSequence"},
{0X00189542, "SQ", "XRayAcquisitionDoseSequence"},
{0X00189543, "FD", "XRaySourceIsocenterPrimaryAngle"},
{0X00189544, "FD", "XRaySourceIsocenterSecondaryAngle"},
{0X00189545, "FD", "BreastSupportIsocenterPrimaryAngle"},
{0X00189546, "FD", "BreastSupportIsocenterSecondaryAngle"},
{0X00189547, "FD", "BreastSupportXPositionToIsocenter"},
{0X00189548, "FD", "BreastSupportYPositionToIsocenter"},
{0X00189549, "FD", "BreastSupportZPositionToIsocenter"},
{0X00189550, "FD", "DetectorIsocenterPrimaryAngle"},
{0X00189551, "FD", "DetectorIsocenterSecondaryAngle"},
{0X00189552, "FD", "DetectorXPositionToIsocenter"},
{0X00189553, "FD", "DetectorYPositionToIsocenter"},
{0X00189554, "FD", "DetectorZPositionToIsocenter"},
{0X00189555, "SQ", "XRayGridSequence"},
{0X00189556, "SQ", "XRayFilterSequence"},
{0X00189557, "FD", "DetectorActiveAreaTLHCPosition"},
{0X00189558, "FD", "DetectorActiveAreaOrientation"},
{0X00189559, "CS", "PositionerPrimaryAngleDirection"},
{0X00189601, "SQ", "DiffusionBMatrixSequence"},
{0X00189602, "FD", "DiffusionBValueXX"},
{0X00189603, "FD", "DiffusionBValueXY"},
{0X00189604, "FD", "DiffusionBValueXZ"},
{0X00189605, "FD", "DiffusionBValueYY"},
{0X00189606, "FD", "DiffusionBValueYZ"},
{0X00189607, "FD", "DiffusionBValueZZ"},
{0X00189621, "SQ", "FunctionalMRSequence"},
{0X00189622, "CS", "FunctionalSettlingPhaseFramesPresent"},
{0X00189623, "DT", "FunctionalSyncPulse"},
{0X00189624, "CS", "SettlingPhaseFrame"},
{0X00189701, "DT", "DecayCorrectionDateTime"},
{0X00189715, "FD", "StartDensityThreshold"},
{0X00189716, "FD", "StartRelativeDensityDifferenceThreshold"},
{0X00189717, "FD", "StartCardiacTriggerCountThreshold"},
{0X00189718, "FD", "StartRespiratoryTriggerCountThreshold"},
{0X00189719, "FD", "TerminationCountsThreshold"},
{0X00189720, "FD", "TerminationDensityThreshold"},
{0X00189721, "FD", "TerminationRelativeDensityThreshold"},
{0X00189722, "FD", "TerminationTimeThreshold"},
{0X00189723, "FD", "TerminationCardiacTriggerCountThreshold"},
{0X00189724, "FD", "TerminationRespiratoryTriggerCountThreshold"},
{0X00189725, "CS", "DetectorGeometry"},
{0X00189726, "FD", "TransverseDetectorSeparation"},
{0X00189727, "FD", "AxialDetectorDimension"},
{0X00189729, "US", "RadiopharmaceuticalAgentNumber"},
{0X00189732, "SQ", "PETFrameAcquisitionSequence"},
{0X00189733, "SQ", "PETDetectorMotionDetailsSequence"},
{0X00189734, "SQ", "PETTableDynamicsSequence"},
{0X00189735, "SQ", "PETPositionSequence"},
{0X00189736, "SQ", "PETFrameCorrectionFactorsSequence"},
{0X00189737, "SQ", "RadiopharmaceuticalUsageSequence"},
{0X00189738, "CS", "AttenuationCorrectionSource"},
{0X00189739, "US", "NumberOfIterations"},
{0X00189740, "US", "NumberOfSubsets"},
{0X00189749, "SQ", "PETReconstructionSequence"},
{0X00189751, "SQ", "PETFrameTypeSequence"},
{0X00189755, "CS", "TimeOfFlightInformationUsed"},
{0X00189756, "CS", "ReconstructionType"},
{0X00189758, "CS", "DecayCorrected"},
{0X00189759, "CS", "AttenuationCorrected"},
{0X00189760, "CS", "ScatterCorrected"},
{0X00189761, "CS", "DeadTimeCorrected"},
{0X00189762, "CS", "GantryMotionCorrected"},
{0X00189763, "CS", "PatientMotionCorrected"},
{0X00189764, "CS", "CountLossNormalizationCorrected"},
{0X00189765, "CS", "RandomsCorrected"},
{0X00189766, "CS", "NonUniformRadialSamplingCorrected"},
{0X00189767, "CS", "SensitivityCalibrated"},
{0X00189768, "CS", "DetectorNormalizationCorrection"},
{0X00189769, "CS", "IterativeReconstructionMethod"},
{0X00189770, "CS", "AttenuationCorrectionTemporalRelationship"},
{0X00189771, "SQ", "PatientPhysiologicalStateSequence"},
{0X00189772, "SQ", "PatientPhysiologicalStateCodeSequence"},
{0X00189801, "FD", "DepthsOfFocus"},
{0X00189803, "SQ", "ExcludedIntervalsSequence"},
{0X00189804, "DT", "ExclusionStartDateTime"},
{0X00189805, "FD", "ExclusionDuration"},
{0X00189806, "SQ", "USImageDescriptionSequence"},
{0X00189807, "SQ", "ImageDataTypeSequence"},
{0X00189808, "CS", "DataType"},
{0X00189809, "SQ", "TransducerScanPatternCodeSequence"},
{0X0018980B, "CS", "AliasedDataType"},
{0X0018980C, "CS", "PositionMeasuringDeviceUsed"},
{0X0018980D, "SQ", "TransducerGeometryCodeSequence"},
{0X0018980E, "SQ", "TransducerBeamSteeringCodeSequence"},
{0X0018980F, "SQ", "TransducerApplicationCodeSequence"},
{0X00189810, "US", "ZeroVelocityPixelValue"},
{0X00189900, "LO", "ReferenceLocationLabel"},
{0X00189901, "UT", "ReferenceLocationDescription"},
{0X00189902, "SQ", "ReferenceBasisCodeSequence"},
{0X00189903, "SQ", "ReferenceGeometryCodeSequence"},
{0X00189904, "DS", "OffsetDistance"},
{0X00189905, "CS", "OffsetDirection"},
{0X00189906, "SQ", "PotentialScheduledProtocolCodeSequence"},
{0X00189907, "SQ", "PotentialRequestedProcedureCodeSequence"},
{0X00189908, "UC", "PotentialReasonsForProcedure"},
{0X00189909, "SQ", "PotentialReasonsForProcedureCodeSequence"},
{0X0018990A, "UC", "PotentialDiagnosticTasks"},
{0X0018990B, "SQ", "ContraindicationsCodeSequence"},
{0X0018990C, "SQ", "ReferencedDefinedProtocolSequence"},
{0X0018990D, "SQ", "ReferencedPerformedProtocolSequence"},
{0X0018990E, "SQ", "PredecessorProtocolSequence"},
{0X0018990F, "UT", "ProtocolPlanningInformation"},
{0X00189910, "UT", "ProtocolDesignRationale"},
{0X00189911, "SQ", "PatientSpecificationSequence"},
{0X00189912, "SQ", "ModelSpecificationSequence"},
{0X00189913, "SQ", "ParametersSpecificationSequence"},
{0X00189914, "SQ", "InstructionSequence"},
{0X00189915, "US", "InstructionIndex"},
{0X00189916, "LO", "InstructionText"},
{0X00189917, "UT", "InstructionDescription"},
{0X00189918, "CS", "InstructionPerformedFlag"},
{0X00189919, "DT", "InstructionPerformedDateTime"},
{0X0018991A, "UT", "InstructionPerformanceComment"},
{0X0018991B, "SQ", "PatientPositioningInstructionSequence"},
{0X0018991C, "SQ", "PositioningMethodCodeSequence"},
{0X0018991D, "SQ", "PositioningLandmarkSequence"},
{0X0018991E, "UI", "TargetFrameOfReferenceUID"},
{0X0018991F, "SQ", "AcquisitionProtocolElementSpecificationSequence"},
{0X00189920, "SQ", "AcquisitionProtocolElementSequence"},
{0X00189921, "US", "ProtocolElementNumber"},
{0X00189922, "LO", "ProtocolElementName"},
{0X00189923, "UT", "ProtocolElementCharacteristicsSummary"},
{0X00189924, "UT", "ProtocolElementPurpose"},
{0X00189930, "CS", "AcquisitionMotion"},
{0X00189931, "SQ", "AcquisitionStartLocationSequence"},
{0X00189932, "SQ", "AcquisitionEndLocationSequence"},
{0X00189933, "SQ", "ReconstructionProtocolElementSpecificationSequence"},
{0X00189934, "SQ", "ReconstructionProtocolElementSequence"},
{0X00189935, "SQ", "StorageProtocolElementSpecificationSequence"},
{0X00189936, "SQ", "StorageProtocolElementSequence"},
{0X00189937, "LO", "RequestedSeriesDescription"},
{0X00189938, "US", "SourceAcquisitionProtocolElementNumber"},
{0X00189939, "US", "SourceAcquisitionBeamNumber"},
{0X0018993A, "US", "SourceReconstructionProtocolElementNumber"},
{0X0018993B, "SQ", "ReconstructionStartLocationSequence"},
{0X0018993C, "SQ", "ReconstructionEndLocationSequence"},
{0X0018993D, "SQ", "ReconstructionAlgorithmSequence"},
{0X0018993E, "SQ", "ReconstructionTargetCenterLocationSequence"},
{0X00189941, "UT", "ImageFilterDescription"},
{0X00189942, "FD", "CTDIvolNotificationTrigger"},
{0X00189943, "FD", "DLPNotificationTrigger"},
{0X00189944, "CS", "AutoKVPSelectionType"},
{0X00189945, "FD", "AutoKVPUpperBound"},
{0X00189946, "FD", "AutoKVPLowerBound"},
{0X00189947, "CS", "ProtocolDefinedPatientPosition"},
{0X0018A001, "SQ", "ContributingEquipmentSequence"},
{0X0018A002, "DT", "ContributionDateTime"},
{0X0018A003, "ST", "ContributionDescription"},
{0X0020000D, "UI", "StudyInstanceUID"},
{0X0020000E, "UI", "SeriesInstanceUID"},
{0X00200010, "SH", "StudyID"},
{0X00200011, "IS", "SeriesNumber"},
{0X00200012, "IS", "AcquisitionNumber"},
{0X00200013, "IS", "InstanceNumber"},
{0X00200014, "IS", "IsotopeNumber"},
{0X00200015, "IS", "PhaseNumber"},
{0X00200016, "IS", "IntervalNumber"},
{0X00200017, "IS", "TimeSlotNumber"},
{0X00200018, "IS", "AngleNumber"},
{0X00200019, "IS", "ItemNumber"},
{0X00200020, "CS", "PatientOrientation"},
{0X00200022, "IS", "OverlayNumber"},
{0X00200024, "IS", "CurveNumber"},
{0X00200026, "IS", "LUTNumber"},
{0X00200030, "DS", "ImagePosition"},
{0X00200032, "DS", "ImagePositionPatient"},
{0X00200035, "DS", "ImageOrientation"},
{0X00200037, "DS", "ImageOrientationPatient"},
{0X00200050, "DS", "Location"},
{0X00200052, "UI", "FrameOfReferenceUID"},
{0X00200060, "CS", "Laterality"},
{0X00200062, "CS", "ImageLaterality"},
{0X00200070, "LO", "ImageGeometryType"},
{0X00200080, "CS", "MaskingImage"},
{0X002000AA, "IS", "ReportNumber"},
{0X00200100, "IS", "TemporalPositionIdentifier"},
{0X00200105, "IS", "NumberOfTemporalPositions"},
{0X00200110, "DS", "TemporalResolution"},
{0X00200200, "UI", "SynchronizationFrameOfReferenceUID"},
{0X00200242, "UI", "SOPInstanceUIDOfConcatenationSource"},
{0X00201000, "IS", "SeriesInStudy"},
{0X00201001, "IS", "AcquisitionsInSeries"},
{0X00201002, "IS", "ImagesInAcquisition"},
{0X00201003, "IS", "ImagesInSeries"},
{0X00201004, "IS", "AcquisitionsInStudy"},
{0X00201005, "IS", "ImagesInStudy"},
{0X00201020, "LO", "Reference"},
{0X0020103F, "LO", "TargetPositionReferenceIndicator"},
{0X00201040, "LO", "PositionReferenceIndicator"},
{0X00201041, "DS", "SliceLocation"},
{0X00201070, "IS", "OtherStudyNumbers"},
{0X00201200, "IS", "NumberOfPatientRelatedStudies"},
{0X00201202, "IS", "NumberOfPatientRelatedSeries"},
{0X00201204, "IS", "NumberOfPatientRelatedInstances"},
{0X00201206, "IS", "NumberOfStudyRelatedSeries"},
{0X00201208, "IS", "NumberOfStudyRelatedInstances"},
{0X00201209, "IS", "NumberOfSeriesRelatedInstances"},
{0X00203401, "CS", "ModifyingDeviceID"},
{0X00203402, "CS", "ModifiedImageID"},
{0X00203403, "DA", "ModifiedImageDate"},
{0X00203404, "LO", "ModifyingDeviceManufacturer"},
{0X00203405, "TM", "ModifiedImageTime"},
{0X00203406, "LO", "ModifiedImageDescription"},
{0X00204000, "LT", "ImageComments"},
{0X00205000, "AT", "OriginalImageIdentification"},
{0X00205002, "LO", "OriginalImageIdentificationNomenclature"},
{0X00209056, "SH", "StackID"},
{0X00209057, "UL", "InStackPositionNumber"},
{0X00209071, "SQ", "FrameAnatomySequence"},
{0X00209072, "CS", "FrameLaterality"},
{0X00209111, "SQ", "FrameContentSequence"},
{0X00209113, "SQ", "PlanePositionSequence"},
{0X00209116, "SQ", "PlaneOrientationSequence"},
{0X00209128, "UL", "TemporalPositionIndex"},
{0X00209153, "FD", "NominalCardiacTriggerDelayTime"},
{0X00209154, "FL", "NominalCardiacTriggerTimePriorToRPeak"},
{0X00209155, "FL", "ActualCardiacTriggerTimePriorToRPeak"},
{0X00209156, "US", "FrameAcquisitionNumber"},
{0X00209157, "UL", "DimensionIndexValues"},
{0X00209158, "LT", "FrameComments"},
{0X00209161, "UI", "ConcatenationUID"},
{0X00209162, "US", "InConcatenationNumber"},
{0X00209163, "US", "InConcatenationTotalNumber"},
{0X00209164, "UI", "DimensionOrganizationUID"},
{0X00209165, "AT", "DimensionIndexPointer"},
{0X00209167, "AT", "FunctionalGroupPointer"},
{0X00209170, "SQ", "UnassignedSharedConvertedAttributesSequence"},
{0X00209171, "SQ", "UnassignedPerFrameConvertedAttributesSequence"},
{0X00209172, "SQ", "ConversionSourceAttributesSequence"},
{0X00209213, "LO", "DimensionIndexPrivateCreator"},
{0X00209221, "SQ", "DimensionOrganizationSequence"},
{0X00209222, "SQ", "DimensionIndexSequence"},
{0X00209228, "UL", "ConcatenationFrameOffsetNumber"},
{0X00209238, "LO", "FunctionalGroupPrivateCreator"},
{0X00209241, "FL", "NominalPercentageOfCardiacPhase"},
{0X00209245, "FL", "NominalPercentageOfRespiratoryPhase"},
{0X00209246, "FL", "StartingRespiratoryAmplitude"},
{0X00209247, "CS", "StartingRespiratoryPhase"},
{0X00209248, "FL", "EndingRespiratoryAmplitude"},
{0X00209249, "CS", "EndingRespiratoryPhase"},
{0X00209250, "CS", "RespiratoryTriggerType"},
{0X00209251, "FD", "RRIntervalTimeNominal"},
{0X00209252, "FD", "ActualCardiacTriggerDelayTime"},
{0X00209253, "SQ", "RespiratorySynchronizationSequence"},
{0X00209254, "FD", "RespiratoryIntervalTime"},
{0X00209255, "FD", "NominalRespiratoryTriggerDelayTime"},
{0X00209256, "FD", "RespiratoryTriggerDelayThreshold"},
{0X00209257, "FD", "ActualRespiratoryTriggerDelayTime"},
{0X00209301, "FD", "ImagePositionVolume"},
{0X00209302, "FD", "ImageOrientationVolume"},
{0X00209307, "CS", "UltrasoundAcquisitionGeometry"},
{0X00209308, "FD", "ApexPosition"},
{0X00209309, "FD", "VolumeToTransducerMappingMatrix"},
{0X0020930A, "FD", "VolumeToTableMappingMatrix"},
{0X0020930B, "CS", "VolumeToTransducerRelationship"},
{0X0020930C, "CS", "PatientFrameOfReferenceSource"},
{0X0020930D, "FD", "TemporalPositionTimeOffset"},
{0X0020930E, "SQ", "PlanePositionVolumeSequence"},
{0X0020930F, "SQ", "PlaneOrientationVolumeSequence"},
{0X00209310, "SQ", "TemporalPositionSequence"},
{0X00209311, "CS", "DimensionOrganizationType"},
{0X00209312, "UI", "VolumeFrameOfReferenceUID"},
{0X00209313, "UI", "TableFrameOfReferenceUID"},
{0X00209421, "LO", "DimensionDescriptionLabel"},
{0X00209450, "SQ", "PatientOrientationInFrameSequence"},
{0X00209453, "LO", "FrameLabel"},
{0X00209518, "US", "AcquisitionIndex"},
{0X00209529, "SQ", "ContributingSOPInstancesReferenceSequence"},
{0X00209536, "US", "ReconstructionIndex"},
{0X00220001, "US", "LightPathFilterPassThroughWavelength"},
{0X00220002, "US", "LightPathFilterPassBand"},
{0X00220003, "US", "ImagePathFilterPassThroughWavelength"},
{0X00220004, "US", "ImagePathFilterPassBand"},
{0X00220005, "CS", "PatientEyeMovementCommanded"},
{0X00220006, "SQ", "PatientEyeMovementCommandCodeSequence"},
{0X00220007, "FL", "SphericalLensPower"},
{0X00220008, "FL", "CylinderLensPower"},
{0X00220009, "FL", "CylinderAxis"},
{0X0022000A, "FL", "EmmetropicMagnification"},
{0X0022000B, "FL", "IntraOcularPressure"},
{0X0022000C, "FL", "HorizontalFieldOfView"},
{0X0022000D, "CS", "PupilDilated"},
{0X0022000E, "FL", "DegreeOfDilation"},
{0X00220010, "FL", "StereoBaselineAngle"},
{0X00220011, "FL", "StereoBaselineDisplacement"},
{0X00220012, "FL", "StereoHorizontalPixelOffset"},
{0X00220013, "FL", "StereoVerticalPixelOffset"},
{0X00220014, "FL", "StereoRotation"},
{0X00220015, "SQ", "AcquisitionDeviceTypeCodeSequence"},
{0X00220016, "SQ", "IlluminationTypeCodeSequence"},
{0X00220017, "SQ", "LightPathFilterTypeStackCodeSequence"},
{0X00220018, "SQ", "ImagePathFilterTypeStackCodeSequence"},
{0X00220019, "SQ", "LensesCodeSequence"},
{0X0022001A, "SQ", "ChannelDescriptionCodeSequence"},
{0X0022001B, "SQ", "RefractiveStateSequence"},
{0X0022001C, "SQ", "MydriaticAgentCodeSequence"},
{0X0022001D, "SQ", "RelativeImagePositionCodeSequence"},
{0X0022001E, "FL", "CameraAngleOfView"},
{0X00220020, "SQ", "StereoPairsSequence"},
{0X00220021, "SQ", "LeftImageSequence"},
{0X00220022, "SQ", "RightImageSequence"},
{0X00220028, "CS", "StereoPairsPresent"},
{0X00220030, "FL", "AxialLengthOfTheEye"},
{0X00220031, "SQ", "OphthalmicFrameLocationSequence"},
{0X00220032, "FL", "ReferenceCoordinates"},
{0X00220035, "FL", "DepthSpatialResolution"},
{0X00220036, "FL", "MaximumDepthDistortion"},
{0X00220037, "FL", "AlongScanSpatialResolution"},
{0X00220038, "FL", "MaximumAlongScanDistortion"},
{0X00220039, "CS", "OphthalmicImageOrientation"},
{0X00220041, "FL", "DepthOfTransverseImage"},
{0X00220042, "SQ", "MydriaticAgentConcentrationUnitsSequence"},
{0X00220048, "FL", "AcrossScanSpatialResolution"},
{0X00220049, "FL", "MaximumAcrossScanDistortion"},
{0X0022004E, "DS", "MydriaticAgentConcentration"},
{0X00220055, "FL", "IlluminationWaveLength"},
{0X00220056, "FL", "IlluminationPower"},
{0X00220057, "FL", "IlluminationBandwidth"},
{0X00220058, "SQ", "MydriaticAgentSequence"},
{0X00221007, "SQ", "OphthalmicAxialMeasurementsRightEyeSequence"},
{0X00221008, "SQ", "OphthalmicAxialMeasurementsLeftEyeSequence"},
{0X00221009, "CS", "OphthalmicAxialMeasurementsDeviceType"},
{0X00221010, "CS", "OphthalmicAxialLengthMeasurementsType"},
{0X00221012, "SQ", "OphthalmicAxialLengthSequence"},
{0X00221019, "FL", "OphthalmicAxialLength"},
{0X00221024, "SQ", "LensStatusCodeSequence"},
{0X00221025, "SQ", "VitreousStatusCodeSequence"},
{0X00221028, "SQ", "IOLFormulaCodeSequence"},
{0X00221029, "LO", "IOLFormulaDetail"},
{0X00221033, "FL", "KeratometerIndex"},
{0X00221035, "SQ", "SourceOfOphthalmicAxialLengthCodeSequence"},
{0X00221036, "SQ", "SourceOfCornealSizeDataCodeSequence"},
{0X00221037, "FL", "TargetRefraction"},
{0X00221039, "CS", "RefractiveProcedureOccurred"},
{0X00221040, "SQ", "RefractiveSurgeryTypeCodeSequence"},
{0X00221044, "SQ", "OphthalmicUltrasoundMethodCodeSequence"},
{0X00221045, "SQ", "SurgicallyInducedAstigmatismSequence"},
{0X00221046, "CS", "TypeOfOpticalCorrection"},
{0X00221047, "SQ", "ToricIOLPowerSequence"},
{0X00221048, "SQ", "PredictedToricErrorSequence"},
{0X00221049, "CS", "PreSelectedForImplantation"},
{0X0022104A, "SQ", "ToricIOLPowerForExactEmmetropiaSequence"},
{0X0022104B, "SQ", "ToricIOLPowerForExactTargetRefractionSequence"},
{0X00221050, "SQ", "OphthalmicAxialLengthMeasurementsSequence"},
{0X00221053, "FL", "IOLPower"},
{0X00221054, "FL", "PredictedRefractiveError"},
{0X00221059, "FL", "OphthalmicAxialLengthVelocity"},
{0X00221065, "LO", "LensStatusDescription"},
{0X00221066, "LO", "VitreousStatusDescription"},
{0X00221090, "SQ", "IOLPowerSequence"},
{0X00221092, "SQ", "LensConstantSequence"},
{0X00221093, "LO", "IOLManufacturer"},
{0X00221094, "LO", "LensConstantDescription"},
{0X00221095, "LO", "ImplantName"},
{0X00221096, "SQ", "KeratometryMeasurementTypeCodeSequence"},
{0X00221097, "LO", "ImplantPartNumber"},
{0X00221100, "SQ", "ReferencedOphthalmicAxialMeasurementsSequence"},
{0X00221101, "SQ", "OphthalmicAxialLengthMeasurementsSegmentNameCodeSequence"},
{0X00221103, "SQ", "RefractiveErrorBeforeRefractiveSurgeryCodeSequence"},
{0X00221121, "FL", "IOLPowerForExactEmmetropia"},
{0X00221122, "FL", "IOLPowerForExactTargetRefraction"},
{0X00221125, "SQ", "AnteriorChamberDepthDefinitionCodeSequence"},
{0X00221127, "SQ", "LensThicknessSequence"},
{0X00221128, "SQ", "AnteriorChamberDepthSequence"},
{0X0022112A, "SQ", "CalculationCommentSequence"},
{0X0022112B, "CS", "CalculationCommentType"},
{0X0022112C, "LT", "CalculationComment"},
{0X00221130, "FL", "LensThickness"},
{0X00221131, "FL", "AnteriorChamberDepth"},
{0X00221132, "SQ", "SourceOfLensThicknessDataCodeSequence"},
{0X00221133, "SQ", "SourceOfAnteriorChamberDepthDataCodeSequence"},
{0X00221134, "SQ", "SourceOfRefractiveMeasurementsSequence"},
{0X00221135, "SQ", "SourceOfRefractiveMeasurementsCodeSequence"},
{0X00221140, "CS", "OphthalmicAxialLengthMeasurementModified"},
{0X00221150, "SQ", "OphthalmicAxialLengthDataSourceCodeSequence"},
{0X00221153, "SQ", "OphthalmicAxialLengthAcquisitionMethodCodeSequence"},
{0X00221155, "FL", "SignalToNoiseRatio"},
{0X00221159, "LO", "OphthalmicAxialLengthDataSourceDescription"},
{0X00221210, "SQ", "OphthalmicAxialLengthMeasurementsTotalLengthSequence"},
{0X00221211, "SQ", "OphthalmicAxialLengthMeasurementsSegmentalLengthSequence"},
{0X00221212, "SQ", "OphthalmicAxialLengthMeasurementsLengthSummationSequence"},
{0X00221220, "SQ", "UltrasoundOphthalmicAxialLengthMeasurementsSequence"},
{0X00221225, "SQ", "OpticalOphthalmicAxialLengthMeasurementsSequence"},
{0X00221230, "SQ", "UltrasoundSelectedOphthalmicAxialLengthSequence"},
{0X00221250, "SQ", "OphthalmicAxialLengthSelectionMethodCodeSequence"},
{0X00221255, "SQ", "OpticalSelectedOphthalmicAxialLengthSequence"},
{0X00221257, "SQ", "SelectedSegmentalOphthalmicAxialLengthSequence"},
{0X00221260, "SQ", "SelectedTotalOphthalmicAxialLengthSequence"},
{0X00221262, "SQ", "OphthalmicAxialLengthQualityMetricSequence"},
{0X00221265, "SQ", "OphthalmicAxialLengthQualityMetricTypeCodeSequence"},
{0X00221273, "LO", "OphthalmicAxialLengthQualityMetricTypeDescription"},
{0X00221300, "SQ", "IntraocularLensCalculationsRightEyeSequence"},
{0X00221310, "SQ", "IntraocularLensCalculationsLeftEyeSequence"},
{0X00221330, "SQ", "ReferencedOphthalmicAxialLengthMeasurementQCImageSequence"},
{0X00221415, "CS", "OphthalmicMappingDeviceType"},
{0X00221420, "SQ", "AcquisitionMethodCodeSequence"},
{0X00221423, "SQ", "AcquisitionMethodAlgorithmSequence"},
{0X00221436, "SQ", "OphthalmicThicknessMapTypeCodeSequence"},
{0X00221443, "SQ", "OphthalmicThicknessMappingNormalsSequence"},
{0X00221445, "SQ", "RetinalThicknessDefinitionCodeSequence"},
{0X00221450, "SQ", "PixelValueMappingToCodedConceptSequence"},
{0X00221452, "US", "MappedPixelValue"},
{0X00221454, "LO", "PixelValueMappingExplanation"},
{0X00221458, "SQ", "OphthalmicThicknessMapQualityThresholdSequence"},
{0X00221460, "FL", "OphthalmicThicknessMapThresholdQualityRating"},
{0X00221463, "FL", "AnatomicStructureReferencePoint"},
{0X00221465, "SQ", "RegistrationToLocalizerSequence"},
{0X00221466, "CS", "RegisteredLocalizerUnits"},
{0X00221467, "FL", "RegisteredLocalizerTopLeftHandCorner"},
{0X00221468, "FL", "RegisteredLocalizerBottomRightHandCorner"},
{0X00221470, "SQ", "OphthalmicThicknessMapQualityRatingSequence"},
{0X00221472, "SQ", "RelevantOPTAttributesSequence"},
{0X00221512, "SQ", "TransformationMethodCodeSequence"},
{0X00221513, "SQ", "TransformationAlgorithmSequence"},
{0X00221515, "CS", "OphthalmicAxialLengthMethod"},
{0X00221517, "FL", "OphthalmicFOV"},
{0X00221518, "SQ", "TwoDimensionalToThreeDimensionalMapSequence"},
{0X00221525, "SQ", "WideFieldOphthalmicPhotographyQualityRatingSequence"},
{0X00221526, "SQ", "WideFieldOphthalmicPhotographyQualityThresholdSequence"},
{0X00221527, "FL", "WideFieldOphthalmicPhotographyThresholdQualityRating"},
{0X00221528, "FL", "XCoordinatesCenterPixelViewAngle"},
{0X00221529, "FL", "YCoordinatesCenterPixelViewAngle"},
{0X00221530, "UL", "NumberOfMapPoints"},
{0X00221531, "OF", "TwoDimensionalToThreeDimensionalMapData"},
{0X00221612, "SQ", "DerivationAlgorithmSequence"},
{0X00221615, "SQ", "OphthalmicImageTypeCodeSequence"},
{0X00221616, "LO", "OphthalmicImageTypeDescription"},
{0X00221618, "SQ", "ScanPatternTypeCodeSequence"},
{0X00221620, "SQ", "ReferencedSurfaceMeshIdentificationSequence"},
{0X00221622, "CS", "OphthalmicVolumetricPropertiesFlag"},
{0X00221624, "FL", "OphthalmicAnatomicReferencePointXCoordinate"},
{0X00221626, "FL", "OphthalmicAnatomicReferencePointYCoordinate"},
{0X00221628, "SQ", "OphthalmicEnFaceImageQualityRatingSequence"},
{0X00221630, "DS", "QualityThreshold"},
{0X00221640, "SQ", "OCTBscanAnalysisAcquisitionParametersSequence"},
{0X00221642, "UL", "NumberOfBscansPerFrame"},
{0X00221643, "FL", "BscanSlabThickness"},
{0X00221644, "FL", "DistanceBetweenBscanSlabs"},
{0X00221645, "FL", "BscanCycleTime"},
{0X00221646, "FL", "BscanCycleTimeVector"},
{0X00221649, "FL", "AscanRate"},
{0X00221650, "FL", "BscanRate"},
{0X00221658, "UL", "SurfaceMeshZPixelOffset"},
{0X00240010, "FL", "VisualFieldHorizontalExtent"},
{0X00240011, "FL", "VisualFieldVerticalExtent"},
{0X00240012, "CS", "VisualFieldShape"},
{0X00240016, "SQ", "ScreeningTestModeCodeSequence"},
{0X00240018, "FL", "MaximumStimulusLuminance"},
{0X00240020, "FL", "BackgroundLuminance"},
{0X00240021, "SQ", "StimulusColorCodeSequence"},
{0X00240024, "SQ", "BackgroundIlluminationColorCodeSequence"},
{0X00240025, "FL", "StimulusArea"},
{0X00240028, "FL", "StimulusPresentationTime"},
{0X00240032, "SQ", "FixationSequence"},
{0X00240033, "SQ", "FixationMonitoringCodeSequence"},
{0X00240034, "SQ", "VisualFieldCatchTrialSequence"},
{0X00240035, "US", "FixationCheckedQuantity"},
{0X00240036, "US", "PatientNotProperlyFixatedQuantity"},
{0X00240037, "CS", "PresentedVisualStimuliDataFlag"},
{0X00240038, "US", "NumberOfVisualStimuli"},
{0X00240039, "CS", "ExcessiveFixationLossesDataFlag"},
{0X00240040, "CS", "ExcessiveFixationLosses"},
{0X00240042, "US", "StimuliRetestingQuantity"},
{0X00240044, "LT", "CommentsOnPatientPerformanceOfVisualField"},
{0X00240045, "CS", "FalseNegativesEstimateFlag"},
{0X00240046, "FL", "FalseNegativesEstimate"},
{0X00240048, "US", "NegativeCatchTrialsQuantity"},
{0X00240050, "US", "FalseNegativesQuantity"},
{0X00240051, "CS", "ExcessiveFalseNegativesDataFlag"},
{0X00240052, "CS", "ExcessiveFalseNegatives"},
{0X00240053, "CS", "FalsePositivesEstimateFlag"},
{0X00240054, "FL", "FalsePositivesEstimate"},
{0X00240055, "CS", "CatchTrialsDataFlag"},
{0X00240056, "US", "PositiveCatchTrialsQuantity"},
{0X00240057, "CS", "TestPointNormalsDataFlag"},
{0X00240058, "SQ", "TestPointNormalsSequence"},
{0X00240059, "CS", "GlobalDeviationProbabilityNormalsFlag"},
{0X00240060, "US", "FalsePositivesQuantity"},
{0X00240061, "CS", "ExcessiveFalsePositivesDataFlag"},
{0X00240062, "CS", "ExcessiveFalsePositives"},
{0X00240063, "CS", "VisualFieldTestNormalsFlag"},
{0X00240064, "SQ", "ResultsNormalsSequence"},
{0X00240065, "SQ", "AgeCorrectedSensitivityDeviationAlgorithmSequence"},
{0X00240066, "FL", "GlobalDeviationFromNormal"},
{0X00240067, "SQ", "GeneralizedDefectSensitivityDeviationAlgorithmSequence"},
{0X00240068, "FL", "LocalizedDeviationFromNormal"},
{0X00240069, "LO", "PatientReliabilityIndicator"},
{0X00240070, "FL", "VisualFieldMeanSensitivity"},
{0X00240071, "FL", "GlobalDeviationProbability"},
{0X00240072, "CS", "LocalDeviationProbabilityNormalsFlag"},
{0X00240073, "FL", "LocalizedDeviationProbability"},
{0X00240074, "CS", "ShortTermFluctuationCalculated"},
{0X00240075, "FL", "ShortTermFluctuation"},
{0X00240076, "CS", "ShortTermFluctuationProbabilityCalculated"},
{0X00240077, "FL", "ShortTermFluctuationProbability"},
{0X00240078, "CS", "CorrectedLocalizedDeviationFromNormalCalculated"},
{0X00240079, "FL", "CorrectedLocalizedDeviationFromNormal"},
{0X00240080, "CS", "CorrectedLocalizedDeviationFromNormalProbabilityCalculated"},
{0X00240081, "FL", "CorrectedLocalizedDeviationFromNormalProbability"},
{0X00240083, "SQ", "GlobalDeviationProbabilitySequence"},
{0X00240085, "SQ", "LocalizedDeviationProbabilitySequence"},
{0X00240086, "CS", "FovealSensitivityMeasured"},
{0X00240087, "FL", "FovealSensitivity"},
{0X00240088, "FL", "VisualFieldTestDuration"},
{0X00240089, "SQ", "VisualFieldTestPointSequence"},
{0X00240090, "FL", "VisualFieldTestPointXCoordinate"},
{0X00240091, "FL", "VisualFieldTestPointYCoordinate"},
{0X00240092, "FL", "AgeCorrectedSensitivityDeviationValue"},
{0X00240093, "CS", "StimulusResults"},
{0X00240094, "FL", "SensitivityValue"},
{0X00240095, "CS", "RetestStimulusSeen"},
{0X00240096, "FL", "RetestSensitivityValue"},
{0X00240097, "SQ", "VisualFieldTestPointNormalsSequence"},
{0X00240098, "FL", "QuantifiedDefect"},
{0X00240100, "FL", "AgeCorrectedSensitivityDeviationProbabilityValue"},
{0X00240102, "CS", "GeneralizedDefectCorrectedSensitivityDeviationFlag"},
{0X00240103, "FL", "GeneralizedDefectCorrectedSensitivityDeviationValue"},
{0X00240104, "FL", "GeneralizedDefectCorrectedSensitivityDeviationProbabilityValue"},
{0X00240105, "FL", "MinimumSensitivityValue"},
{0X00240106, "CS", "BlindSpotLocalized"},
{0X00240107, "FL", "BlindSpotXCoordinate"},
{0X00240108, "FL", "BlindSpotYCoordinate"},
{0X00240110, "SQ", "VisualAcuityMeasurementSequence"},
{0X00240112, "SQ", "RefractiveParametersUsedOnPatientSequence"},
{0X00240113, "CS", "MeasurementLaterality"},
{0X00240114, "SQ", "OphthalmicPatientClinicalInformationLeftEyeSequence"},
{0X00240115, "SQ", "OphthalmicPatientClinicalInformationRightEyeSequence"},
{0X00240117, "CS", "FovealPointNormativeDataFlag"},
{0X00240118, "FL", "FovealPointProbabilityValue"},
{0X00240120, "CS", "ScreeningBaselineMeasured"},
{0X00240122, "SQ", "ScreeningBaselineMeasuredSequence"},
{0X00240124, "CS", "ScreeningBaselineType"},
{0X00240126, "FL", "ScreeningBaselineValue"},
{0X00240202, "LO", "AlgorithmSource"},
{0X00240306, "LO", "DataSetName"},
{0X00240307, "LO", "DataSetVersion"},
{0X00240308, "LO", "DataSetSource"},
{0X00240309, "LO", "DataSetDescription"},
{0X00240317, "SQ", "VisualFieldTestReliabilityGlobalIndexSequence"},
{0X00240320, "SQ", "VisualFieldGlobalResultsIndexSequence"},
{0X00240325, "SQ", "DataObservationSequence"},
{0X00240338, "CS", "IndexNormalsFlag"},
{0X00240341, "FL", "IndexProbability"},
{0X00240344, "SQ", "IndexProbabilitySequence"},
{0X00280002, "US", "SamplesPerPixel"},
{0X00280003, "US", "SamplesPerPixelUsed"},
{0X00280004, "CS", "PhotometricInterpretation"},
{0X00280005, "US", "ImageDimensions"},
{0X00280006, "US", "PlanarConfiguration"},
{0X00280008, "IS", "NumberOfFrames"},
{0X00280009, "AT", "FrameIncrementPointer"},
{0X0028000A, "AT", "FrameDimensionPointer"},
{0X00280010, "US", "Rows"},
{0X00280011, "US", "Columns"},
{0X00280012, "US", "Planes"},
{0X00280014, "US", "UltrasoundColorDataPresent"},
{0X00280020, "OB", ""},
{0X00280030, "DS", "PixelSpacing"},
{0X00280031, "DS", "ZoomFactor"},
{0X00280032, "DS", "ZoomCenter"},
{0X00280034, "IS", "PixelAspectRatio"},
{0X00280040, "CS", "ImageFormat"},
{0X00280050, "LO", "ManipulatedImage"},
{0X00280051, "CS", "CorrectedImage"},
{0X0028005F, "LO", "CompressionRecognitionCode"},
{0X00280060, "CS", "CompressionCode"},
{0X00280061, "SH", "CompressionOriginator"},
{0X00280062, "LO", "CompressionLabel"},
{0X00280063, "SH", "CompressionDescription"},
{0X00280065, "CS", "CompressionSequence"},
{0X00280066, "AT", "CompressionStepPointers"},
{0X00280068, "US", "RepeatInterval"},
{0X00280069, "US", "BitsGrouped"},
{0X00280070, "US", "PerimeterTable"},
{0X00280071, "US", "PerimeterValue"},
{0X00280080, "US", "PredictorRows"},
{0X00280081, "US", "PredictorColumns"},
{0X00280082, "US", "PredictorConstants"},
{0X00280090, "CS", "BlockedPixels"},
{0X00280091, "US", "BlockRows"},
{0X00280092, "US", "BlockColumns"},
{0X00280093, "US", "RowOverlap"},
{0X00280094, "US", "ColumnOverlap"},
{0X00280100, "US", "BitsAllocated"},
{0X00280101, "US", "BitsStored"},
{0X00280102, "US", "HighBit"},
{0X00280103, "US", "PixelRepresentation"},
{0X00280104, "US", "SmallestValidPixelValue"},
{0X00280105, "US", "LargestValidPixelValue"},
{0X00280106, "US", "SmallestImagePixelValue"},
{0X00280107, "US", "LargestImagePixelValue"},
{0X00280108, "US", "SmallestPixelValueInSeries"},
{0X00280109, "US", "LargestPixelValueInSeries"},
{0X00280110, "US", "SmallestImagePixelValueInPlane"},
{0X00280111, "US", "LargestImagePixelValueInPlane"},
{0X00280120, "US", "PixelPaddingValue"},
{0X00280121, "US", "PixelPaddingRangeLimit"},
{0X00280122, "FL", "FloatPixelPaddingValue"},
{0X00280123, "FD", "DoubleFloatPixelPaddingValue"},
{0X00280124, "FL", "FloatPixelPaddingRangeLimit"},
{0X00280125, "FD", "DoubleFloatPixelPaddingRangeLimit"},
{0X00280200, "US", "ImageLocation"},
{0X00280300, "CS", "QualityControlImage"},
{0X00280301, "CS", "BurnedInAnnotation"},
{0X00280302, "CS", "RecognizableVisualFeatures"},
{0X00280303, "CS", "LongitudinalTemporalInformationModified"},
{0X00280304, "UI", "ReferencedColorPaletteInstanceUID"},
{0X00280400, "LO", "TransformLabel"},
{0X00280401, "LO", "TransformVersionNumber"},
{0X00280402, "US", "NumberOfTransformSteps"},
{0X00280403, "LO", "SequenceOfCompressedData"},
{0X00280404, "AT", "DetailsOfCoefficients"},
{0X00280700, "LO", "DCTLabel"},
{0X00280701, "CS", "DataBlockDescription"},
{0X00280702, "AT", "DataBlock"},
{0X00280710, "US", "NormalizationFactorFormat"},
{0X00280720, "US", "ZonalMapNumberFormat"},
{0X00280721, "AT", "ZonalMapLocation"},
{0X00280722, "US", "ZonalMapFormat"},
{0X00280730, "US", "AdaptiveMapFormat"},
{0X00280740, "US", "CodeNumberFormat"},
{0X00280A02, "CS", "PixelSpacingCalibrationType"},
{0X00280A04, "LO", "PixelSpacingCalibrationDescription"},
{0X00281040, "CS", "PixelIntensityRelationship"},
{0X00281041, "SS", "PixelIntensityRelationshipSign"},
{0X00281050, "DS", "WindowCenter"},
{0X00281051, "DS", "WindowWidth"},
{0X00281052, "DS", "RescaleIntercept"},
{0X00281053, "DS", "RescaleSlope"},
{0X00281054, "LO", "RescaleType"},
{0X00281055, "LO", "WindowCenterWidthExplanation"},
{0X00281056, "CS", "VOILUTFunction"},
{0X00281080, "CS", "GrayScale"},
{0X00281090, "CS", "RecommendedViewingMode"},
{0X00281100, "US", "GrayLookupTableDescriptor"},
{0X00281101, "US", "RedPaletteColorLookupTableDescriptor"},
{0X00281102, "US", "GreenPaletteColorLookupTableDescriptor"},
{0X00281103, "US", "BluePaletteColorLookupTableDescriptor"},
{0X00281104, "US", "AlphaPaletteColorLookupTableDescriptor"},
{0X00281111, "US", "LargeRedPaletteColorLookupTableDescriptor"},
{0X00281112, "US", "LargeGreenPaletteColorLookupTableDescriptor"},
{0X00281113, "US", "LargeBluePaletteColorLookupTableDescriptor"},
{0X00281199, "UI", "PaletteColorLookupTableUID"},
{0X00281200, "US", "GrayLookupTableData"},
{0X00281201, "OW", "RedPaletteColorLookupTableData"},
{0X00281202, "OW", "GreenPaletteColorLookupTableData"},
{0X00281203, "OW", "BluePaletteColorLookupTableData"},
{0X00281204, "OW", "AlphaPaletteColorLookupTableData"},
{0X00281211, "OW", "LargeRedPaletteColorLookupTableData"},
{0X00281212, "OW", "LargeGreenPaletteColorLookupTableData"},
{0X00281213, "OW", "LargeBluePaletteColorLookupTableData"},
{0X00281214, "UI", "LargePaletteColorLookupTableUID"},
{0X00281221, "OW", "SegmentedRedPaletteColorLookupTableData"},
{0X00281222, "OW", "SegmentedGreenPaletteColorLookupTableData"},
{0X00281223, "OW", "SegmentedBluePaletteColorLookupTableData"},
{0X00281224, "OW", "SegmentedAlphaPaletteColorLookupTableData"},
{0X00281230, "SQ", "StoredValueColorRangeSequence"},
{0X00281231, "FD", "MinimumStoredValueMapped"},
{0X00281232, "FD", "MaximumStoredValueMapped"},
{0X00281300, "CS", "BreastImplantPresent"},
{0X00281350, "CS", "PartialView"},
{0X00281351, "ST", "PartialViewDescription"},
{0X00281352, "SQ", "PartialViewCodeSequence"},
{0X0028135A, "CS", "SpatialLocationsPreserved"},
{0X00281401, "SQ", "DataFrameAssignmentSequence"},
{0X00281402, "CS", "DataPathAssignment"},
{0X00281403, "US", "BitsMappedToColorLookupTable"},
{0X00281404, "SQ", "BlendingLUT1Sequence"},
{0X00281405, "CS", "BlendingLUT1TransferFunction"},
{0X00281406, "FD", "BlendingWeightConstant"},
{0X00281407, "US", "BlendingLookupTableDescriptor"},
{0X00281408, "OW", "BlendingLookupTableData"},
{0X0028140B, "SQ", "EnhancedPaletteColorLookupTableSequence"},
{0X0028140C, "SQ", "BlendingLUT2Sequence"},
{0X0028140D, "CS", "BlendingLUT2TransferFunction"},
{0X0028140E, "CS", "DataPathID"},
{0X0028140F, "CS", "RGBLUTTransferFunction"},
{0X00281410, "CS", "AlphaLUTTransferFunction"},
{0X00282000, "OB", "ICCProfile"},
{0X00282002, "CS", "ColorSpace"},
{0X00282110, "CS", "LossyImageCompression"},
{0X00282112, "DS", "LossyImageCompressionRatio"},
{0X00282114, "CS", "LossyImageCompressionMethod"},
{0X00283000, "SQ", "ModalityLUTSequence"},
{0X00283002, "US", "LUTDescriptor"},
{0X00283003, "LO", "LUTExplanation"},
{0X00283004, "LO", "ModalityLUTType"},
{0X00283006, "US", "LUTData"},
{0X00283010, "SQ", "VOILUTSequence"},
{0X00283110, "SQ", "SoftcopyVOILUTSequence"},
{0X00284000, "LT", "ImagePresentationComments"},
{0X00285000, "SQ", "BiPlaneAcquisitionSequence"},
{0X00286010, "US", "RepresentativeFrameNumber"},
{0X00286020, "US", "FrameNumbersOfInterest"},
{0X00286022, "LO", "FrameOfInterestDescription"},
{0X00286023, "CS", "FrameOfInterestType"},
{0X00286030, "US", "MaskPointers"},
{0X00286040, "US", "RWavePointer"},
{0X00286100, "SQ", "MaskSubtractionSequence"},
{0X00286101, "CS", "MaskOperation"},
{0X00286102, "US", "ApplicableFrameRange"},
{0X00286110, "US", "MaskFrameNumbers"},
{0X00286112, "US", "ContrastFrameAveraging"},
{0X00286114, "FL", "MaskSubPixelShift"},
{0X00286120, "SS", "TIDOffset"},
{0X00286190, "ST", "MaskOperationExplanation"},
{0X00287000, "SQ", "EquipmentAdministratorSequence"},
{0X00287001, "US", "NumberOfDisplaySubsystems"},
{0X00287002, "US", "CurrentConfigurationID"},
{0X00287003, "US", "DisplaySubsystemID"},
{0X00287004, "SH", "DisplaySubsystemName"},
{0X00287005, "LO", "DisplaySubsystemDescription"},
{0X00287006, "CS", "SystemStatus"},
{0X00287007, "LO", "SystemStatusComment"},
{0X00287008, "SQ", "TargetLuminanceCharacteristicsSequence"},
{0X00287009, "US", "LuminanceCharacteristicsID"},
{0X0028700A, "SQ", "DisplaySubsystemConfigurationSequence"},
{0X0028700B, "US", "ConfigurationID"},
{0X0028700C, "SH", "ConfigurationName"},
{0X0028700D, "LO", "ConfigurationDescription"},
{0X0028700E, "US", "ReferencedTargetLuminanceCharacteristicsID"},
{0X0028700F, "SQ", "QAResultsSequence"},
{0X00287010, "SQ", "DisplaySubsystemQAResultsSequence"},
{0X00287011, "SQ", "ConfigurationQAResultsSequence"},
{0X00287012, "SQ", "MeasurementEquipmentSequence"},
{0X00287013, "CS", "MeasurementFunctions"},
{0X00287014, "CS", "MeasurementEquipmentType"},
{0X00287015, "SQ", "VisualEvaluationResultSequence"},
{0X00287016, "SQ", "DisplayCalibrationResultSequence"},
{0X00287017, "US", "DDLValue"},
{0X00287018, "FL", "CIExyWhitePoint"},
{0X00287019, "CS", "DisplayFunctionType"},
{0X0028701A, "FL", "GammaValue"},
{0X0028701B, "US", "NumberOfLuminancePoints"},
{0X0028701C, "SQ", "LuminanceResponseSequence"},
{0X0028701D, "FL", "TargetMinimumLuminance"},
{0X0028701E, "FL", "TargetMaximumLuminance"},
{0X0028701F, "FL", "LuminanceValue"},
{0X00287020, "LO", "LuminanceResponseDescription"},
{0X00287021, "CS", "WhitePointFlag"},
{0X00287022, "SQ", "DisplayDeviceTypeCodeSequence"},
{0X00287023, "SQ", "DisplaySubsystemSequence"},
{0X00287024, "SQ", "LuminanceResultSequence"},
{0X00287025, "CS", "AmbientLightValueSource"},
{0X00287026, "CS", "MeasuredCharacteristics"},
{0X00287027, "SQ", "LuminanceUniformityResultSequence"},
{0X00287028, "SQ", "VisualEvaluationTestSequence"},
{0X00287029, "CS", "TestResult"},
{0X0028702A, "LO", "TestResultComment"},
{0X0028702B, "CS", "TestImageValidation"},
{0X0028702C, "SQ", "TestPatternCodeSequence"},
{0X0028702D, "SQ", "MeasurementPatternCodeSequence"},
{0X0028702E, "SQ", "VisualEvaluationMethodCodeSequence"},
{0X00287FE0, "UR", "PixelDataProviderURL"},
{0X00289001, "UL", "DataPointRows"},
{0X00289002, "UL", "DataPointColumns"},
{0X00289003, "CS", "SignalDomainColumns"},
{0X00289099, "US", "LargestMonochromePixelValue"},
{0X00289108, "CS", "DataRepresentation"},
{0X00289110, "SQ", "PixelMeasuresSequence"},
{0X00289132, "SQ", "FrameVOILUTSequence"},
{0X00289145, "SQ", "PixelValueTransformationSequence"},
{0X00289235, "CS", "SignalDomainRows"},
{0X00289411, "FL", "DisplayFilterPercentage"},
{0X00289415, "SQ", "FramePixelShiftSequence"},
{0X00289416, "US", "SubtractionItemID"},
{0X00289422, "SQ", "PixelIntensityRelationshipLUTSequence"},
{0X00289443, "SQ", "FramePixelDataPropertiesSequence"},
{0X00289444, "CS", "GeometricalProperties"},
{0X00289445, "FL", "GeometricMaximumDistortion"},
{0X00289446, "CS", "ImageProcessingApplied"},
{0X00289454, "CS", "MaskSelectionMode"},
{0X00289474, "CS", "LUTFunction"},
{0X00289478, "FL", "MaskVisibilityPercentage"},
{0X00289501, "SQ", "PixelShiftSequence"},
{0X00289502, "SQ", "RegionPixelShiftSequence"},
{0X00289503, "SS", "VerticesOfTheRegion"},
{0X00289505, "SQ", "MultiFramePresentationSequence"},
{0X00289506, "US", "PixelShiftFrameRange"},
{0X00289507, "US", "LUTFrameRange"},
{0X00289520, "DS", "ImageToEquipmentMappingMatrix"},
{0X00289537, "CS", "EquipmentCoordinateSystemIdentification"},
{0X0032000A, "CS", "StudyStatusID"},
{0X0032000C, "CS", "StudyPriorityID"},
{0X00320012, "LO", "StudyIDIssuer"},
{0X00320032, "DA", "StudyVerifiedDate"},
{0X00320033, "TM", "StudyVerifiedTime"},
{0X00320034, "DA", "StudyReadDate"},
{0X00320035, "TM", "StudyReadTime"},
{0X00321000, "DA", "ScheduledStudyStartDate"},
{0X00321001, "TM", "ScheduledStudyStartTime"},
{0X00321010, "DA", "ScheduledStudyStopDate"},
{0X00321011, "TM", "ScheduledStudyStopTime"},
{0X00321020, "LO", "ScheduledStudyLocation"},
{0X00321021, "AE", "ScheduledStudyLocationAETitle"},
{0X00321030, "LO", "ReasonForStudy"},
{0X00321031, "SQ", "RequestingPhysicianIdentificationSequence"},
{0X00321032, "PN", "RequestingPhysician"},
{0X00321033, "LO", "RequestingService"},
{0X00321034, "SQ", "RequestingServiceCodeSequence"},
{0X00321040, "DA", "StudyArrivalDate"},
{0X00321041, "TM", "StudyArrivalTime"},
{0X00321050, "DA", "StudyCompletionDate"},
{0X00321051, "TM", "StudyCompletionTime"},
{0X00321055, "CS", "StudyComponentStatusID"},
{0X00321060, "LO", "RequestedProcedureDescription"},
{0X00321064, "SQ", "RequestedProcedureCodeSequence"},
{0X00321066, "UT", "ReasonForVisit"},
{0X00321067, "SQ", "ReasonForVisitCodeSequence"},
{0X00321070, "LO", "RequestedContrastAgent"},
{0X00324000, "LT", "StudyComments"},
{0X00340001, "SQ", "FlowIdentifierSequence"},
{0X00340002, "OB", "FlowIdentifier"},
{0X00340003, "UI", "FlowTransferSyntaxUID"},
{0X00340004, "UL", "FlowRTPSamplingRate"},
{0X00340005, "OB", "SourceIdentifier"},
{0X00340007, "OB", "FrameOriginTimestamp"},
{0X00340008, "CS", "IncludesImagingSubject"},
{0X00340009, "SQ", "FrameUsefulnessGroupSequence"},
{0X0034000A, "SQ", "RealTimeBulkDataFlowSequence"},
{0X0034000B, "SQ", "CameraPositionGroupSequence"},
{0X0034000C, "CS", "IncludesInformation"},
{0X0034000D, "SQ", "TimeOfFrameGroupSequence"},
{0X00380004, "SQ", "ReferencedPatientAliasSequence"},
{0X00380008, "CS", "VisitStatusID"},
{0X00380010, "LO", "AdmissionID"},
{0X00380011, "LO", "IssuerOfAdmissionID"},
{0X00380014, "SQ", "IssuerOfAdmissionIDSequence"},
{0X00380016, "LO", "RouteOfAdmissions"},
{0X0038001A, "DA", "ScheduledAdmissionDate"},
{0X0038001B, "TM", "ScheduledAdmissionTime"},
{0X0038001C, "DA", "ScheduledDischargeDate"},
{0X0038001D, "TM", "ScheduledDischargeTime"},
{0X0038001E, "LO", "ScheduledPatientInstitutionResidence"},
{0X00380020, "DA", "AdmittingDate"},
{0X00380021, "TM", "AdmittingTime"},
{0X00380030, "DA", "DischargeDate"},
{0X00380032, "TM", "DischargeTime"},
{0X00380040, "LO", "DischargeDiagnosisDescription"},
{0X00380044, "SQ", "DischargeDiagnosisCodeSequence"},
{0X00380050, "LO", "SpecialNeeds"},
{0X00380060, "LO", "ServiceEpisodeID"},
{0X00380061, "LO", "IssuerOfServiceEpisodeID"},
{0X00380062, "LO", "ServiceEpisodeDescription"},
{0X00380064, "SQ", "IssuerOfServiceEpisodeIDSequence"},
{0X00380100, "SQ", "PertinentDocumentsSequence"},
{0X00380101, "SQ", "PertinentResourcesSequence"},
{0X00380102, "LO", "ResourceDescription"},
{0X00380300, "LO", "CurrentPatientLocation"},
{0X00380400, "LO", "PatientInstitutionResidence"},
{0X00380500, "LO", "PatientState"},
{0X00380502, "SQ", "PatientClinicalTrialParticipationSequence"},
{0X00384000, "LT", "VisitComments"},
{0X003A0004, "CS", "WaveformOriginality"},
{0X003A0005, "US", "NumberOfWaveformChannels"},
{0X003A0010, "UL", "NumberOfWaveformSamples"},
{0X003A001A, "DS", "SamplingFrequency"},
{0X003A0020, "SH", "MultiplexGroupLabel"},
{0X003A0200, "SQ", "ChannelDefinitionSequence"},
{0X003A0202, "IS", "WaveformChannelNumber"},
{0X003A0203, "SH", "ChannelLabel"},
{0X003A0205, "CS", "ChannelStatus"},
{0X003A0208, "SQ", "ChannelSourceSequence"},
{0X003A0209, "SQ", "ChannelSourceModifiersSequence"},
{0X003A020A, "SQ", "SourceWaveformSequence"},
{0X003A020C, "LO", "ChannelDerivationDescription"},
{0X003A0210, "DS", "ChannelSensitivity"},
{0X003A0211, "SQ", "ChannelSensitivityUnitsSequence"},
{0X003A0212, "DS", "ChannelSensitivityCorrectionFactor"},
{0X003A0213, "DS", "ChannelBaseline"},
{0X003A0214, "DS", "ChannelTimeSkew"},
{0X003A0215, "DS", "ChannelSampleSkew"},
{0X003A0218, "DS", "ChannelOffset"},
{0X003A021A, "US", "WaveformBitsStored"},
{0X003A0220, "DS", "FilterLowFrequency"},
{0X003A0221, "DS", "FilterHighFrequency"},
{0X003A0222, "DS", "NotchFilterFrequency"},
{0X003A0223, "DS", "NotchFilterBandwidth"},
{0X003A0230, "FL", "WaveformDataDisplayScale"},
{0X003A0231, "US", "WaveformDisplayBackgroundCIELabValue"},
{0X003A0240, "SQ", "WaveformPresentationGroupSequence"},
{0X003A0241, "US", "PresentationGroupNumber"},
{0X003A0242, "SQ", "ChannelDisplaySequence"},
{0X003A0244, "US", "ChannelRecommendedDisplayCIELabValue"},
{0X003A0245, "FL", "ChannelPosition"},
{0X003A0246, "CS", "DisplayShadingFlag"},
{0X003A0247, "FL", "FractionalChannelDisplayScale"},
{0X003A0248, "FL", "AbsoluteChannelDisplayScale"},
{0X003A0300, "SQ", "MultiplexedAudioChannelsDescriptionCodeSequence"},
{0X003A0301, "IS", "ChannelIdentificationCode"},
{0X003A0302, "CS", "ChannelMode"},
{0X003A0310, "UI", "MultiplexGroupUID"},
{0X003A0311, "DS", "PowerlineFrequency"},
{0X003A0312, "SQ", "ChannelImpedanceSequence"},
{0X003A0313, "DS", "ImpedanceValue"},
{0X003A0314, "DT", "ImpedanceMeasurementDateTime"},
{0X003A0315, "DS", "ImpedanceMeasurementFrequency"},
{0X003A0316, "CS", "ImpedanceMeasurementCurrentType"},
{0X00400001, "AE", "ScheduledStationAETitle"},
{0X00400002, "DA", "ScheduledProcedureStepStartDate"},
{0X00400003, "TM", "ScheduledProcedureStepStartTime"},
{0X00400004, "DA", "ScheduledProcedureStepEndDate"},
{0X00400005, "TM", "ScheduledProcedureStepEndTime"},
{0X00400006, "PN", "ScheduledPerformingPhysicianName"},
{0X00400007, "LO", "ScheduledProcedureStepDescription"},
{0X00400008, "SQ", "ScheduledProtocolCodeSequence"},
{0X00400009, "SH", "ScheduledProcedureStepID"},
{0X0040000A, "SQ", "StageCodeSequence"},
{0X0040000B, "SQ", "ScheduledPerformingPhysicianIdentificationSequence"},
{0X00400010, "SH", "ScheduledStationName"},
{0X00400011, "SH", "ScheduledProcedureStepLocation"},
{0X00400012, "LO", "PreMedication"},
{0X00400020, "CS", "ScheduledProcedureStepStatus"},
{0X00400026, "SQ", "OrderPlacerIdentifierSequence"},
{0X00400027, "SQ", "OrderFillerIdentifierSequence"},
{0X00400031, "UT", "LocalNamespaceEntityID"},
{0X00400032, "UT", "UniversalEntityID"},
{0X00400033, "CS", "UniversalEntityIDType"},
{0X00400035, "CS", "IdentifierTypeCode"},
{0X00400036, "SQ", "AssigningFacilitySequence"},
{0X00400039, "SQ", "AssigningJurisdictionCodeSequence"},
{0X0040003A, "SQ", "AssigningAgencyOrDepartmentCodeSequence"},
{0X00400100, "SQ", "ScheduledProcedureStepSequence"},
{0X00400220, "SQ", "ReferencedNonImageCompositeSOPInstanceSequence"},
{0X00400241, "AE", "PerformedStationAETitle"},
{0X00400242, "SH", "PerformedStationName"},
{0X00400243, "SH", "PerformedLocation"},
{0X00400244, "DA", "PerformedProcedureStepStartDate"},
{0X00400245, "TM", "PerformedProcedureStepStartTime"},
{0X00400250, "DA", "PerformedProcedureStepEndDate"},
{0X00400251, "TM", "PerformedProcedureStepEndTime"},
{0X00400252, "CS", "PerformedProcedureStepStatus"},
{0X00400253, "SH", "PerformedProcedureStepID"},
{0X00400254, "LO", "PerformedProcedureStepDescription"},
{0X00400255, "LO", "PerformedProcedureTypeDescription"},
{0X00400260, "SQ", "PerformedProtocolCodeSequence"},
{0X00400261, "CS", "PerformedProtocolType"},
{0X00400270, "SQ", "ScheduledStepAttributesSequence"},
{0X00400275, "SQ", "RequestAttributesSequence"},
{0X00400280, "ST", "CommentsOnThePerformedProcedureStep"},
{0X00400281, "SQ", "PerformedProcedureStepDiscontinuationReasonCodeSequence"},
{0X00400293, "SQ", "QuantitySequence"},
{0X00400294, "DS", "Quantity"},
{0X00400295, "SQ", "MeasuringUnitsSequence"},
{0X00400296, "SQ", "BillingItemSequence"},
{0X00400300, "US", "TotalTimeOfFluoroscopy"},
{0X00400301, "US", "TotalNumberOfExposures"},
{0X00400302, "US", "EntranceDose"},
{0X00400303, "US", "ExposedArea"},
{0X00400306, "DS", "DistanceSourceToEntrance"},
{0X00400307, "DS", "DistanceSourceToSupport"},
{0X0040030E, "SQ", "ExposureDoseSequence"},
{0X00400310, "ST", "CommentsOnRadiationDose"},
{0X00400312, "DS", "XRayOutput"},
{0X00400314, "DS", "HalfValueLayer"},
{0X00400316, "DS", "OrganDose"},
{0X00400318, "CS", "OrganExposed"},
{0X00400320, "SQ", "BillingProcedureStepSequence"},
{0X00400321, "SQ", "FilmConsumptionSequence"},
{0X00400324, "SQ", "BillingSuppliesAndDevicesSequence"},
{0X00400330, "SQ", "ReferencedProcedureStepSequence"},
{0X00400340, "SQ", "PerformedSeriesSequence"},
{0X00400400, "LT", "CommentsOnTheScheduledProcedureStep"},
{0X00400440, "SQ", "ProtocolContextSequence"},
{0X00400441, "SQ", "ContentItemModifierSequence"},
{0X00400500, "SQ", "ScheduledSpecimenSequence"},
{0X0040050A, "LO", "SpecimenAccessionNumber"},
{0X00400512, "LO", "ContainerIdentifier"},
{0X00400513, "SQ", "IssuerOfTheContainerIdentifierSequence"},
{0X00400515, "SQ", "AlternateContainerIdentifierSequence"},
{0X00400518, "SQ", "ContainerTypeCodeSequence"},
{0X0040051A, "LO", "ContainerDescription"},
{0X00400520, "SQ", "ContainerComponentSequence"},
{0X00400550, "SQ", "SpecimenSequence"},
{0X00400551, "LO", "SpecimenIdentifier"},
{0X00400552, "SQ", "SpecimenDescriptionSequenceTrial"},
{0X00400553, "ST", "SpecimenDescriptionTrial"},
{0X00400554, "UI", "SpecimenUID"},
{0X00400555, "SQ", "AcquisitionContextSequence"},
{0X00400556, "ST", "AcquisitionContextDescription"},
{0X00400560, "SQ", "SpecimenDescriptionSequence"},
{0X00400562, "SQ", "IssuerOfTheSpecimenIdentifierSequence"},
{0X0040059A, "SQ", "SpecimenTypeCodeSequence"},
{0X00400600, "LO", "SpecimenShortDescription"},
{0X00400602, "UT", "SpecimenDetailedDescription"},
{0X00400610, "SQ", "SpecimenPreparationSequence"},
{0X00400612, "SQ", "SpecimenPreparationStepContentItemSequence"},
{0X00400620, "SQ", "SpecimenLocalizationContentItemSequence"},
{0X004006FA, "LO", "SlideIdentifier"},
{0X00400710, "SQ", "WholeSlideMicroscopyImageFrameTypeSequence"},
{0X0040071A, "SQ", "ImageCenterPointCoordinatesSequence"},
{0X0040072A, "DS", "XOffsetInSlideCoordinateSystem"},
{0X0040073A, "DS", "YOffsetInSlideCoordinateSystem"},
{0X0040074A, "DS", "ZOffsetInSlideCoordinateSystem"},
{0X004008D8, "SQ", "PixelSpacingSequence"},
{0X004008DA, "SQ", "CoordinateSystemAxisCodeSequence"},
{0X004008EA, "SQ", "MeasurementUnitsCodeSequence"},
{0X004009F8, "SQ", "VitalStainCodeSequenceTrial"},
{0X00401001, "SH", "RequestedProcedureID"},
{0X00401002, "LO", "ReasonForTheRequestedProcedure"},
{0X00401003, "SH", "RequestedProcedurePriority"},
{0X00401004, "LO", "PatientTransportArrangements"},
{0X00401005, "LO", "RequestedProcedureLocation"},
{0X00401006, "SH", "PlacerOrderNumberProcedure"},
{0X00401007, "SH", "FillerOrderNumberProcedure"},
{0X00401008, "LO", "ConfidentialityCode"},
{0X00401009, "SH", "ReportingPriority"},
{0X0040100A, "SQ", "ReasonForRequestedProcedureCodeSequence"},
{0X00401010, "PN", "NamesOfIntendedRecipientsOfResults"},
{0X00401011, "SQ", "IntendedRecipientsOfResultsIdentificationSequence"},
{0X00401012, "SQ", "ReasonForPerformedProcedureCodeSequence"},
{0X00401060, "LO", "RequestedProcedureDescriptionTrial"},
{0X00401101, "SQ", "PersonIdentificationCodeSequence"},
{0X00401102, "ST", "PersonAddress"},
{0X00401103, "LO", "PersonTelephoneNumbers"},
{0X00401104, "LT", "PersonTelecomInformation"},
{0X00401400, "LT", "RequestedProcedureComments"},
{0X00402001, "LO", "ReasonForTheImagingServiceRequest"},
{0X00402004, "DA", "IssueDateOfImagingServiceRequest"},
{0X00402005, "TM", "IssueTimeOfImagingServiceRequest"},
{0X00402006, "SH", "PlacerOrderNumberImagingServiceRequestRetired"},
{0X00402007, "SH", "FillerOrderNumberImagingServiceRequestRetired"},
{0X00402008, "PN", "OrderEnteredBy"},
{0X00402009, "SH", "OrderEntererLocation"},
{0X00402010, "SH", "OrderCallbackPhoneNumber"},
{0X00402011, "LT", "OrderCallbackTelecomInformation"},
{0X00402016, "LO", "PlacerOrderNumberImagingServiceRequest"},
{0X00402017, "LO", "FillerOrderNumberImagingServiceRequest"},
{0X00402400, "LT", "ImagingServiceRequestComments"},
{0X00403001, "LO", "ConfidentialityConstraintOnPatientDataDescription"},
{0X00404001, "CS", "GeneralPurposeScheduledProcedureStepStatus"},
{0X00404002, "CS", "GeneralPurposePerformedProcedureStepStatus"},
{0X00404003, "CS", "GeneralPurposeScheduledProcedureStepPriority"},
{0X00404004, "SQ", "ScheduledProcessingApplicationsCodeSequence"},
{0X00404005, "DT", "ScheduledProcedureStepStartDateTime"},
{0X00404006, "CS", "MultipleCopiesFlag"},
{0X00404007, "SQ", "PerformedProcessingApplicationsCodeSequence"},
{0X00404008, "DT", "ScheduledProcedureStepExpirationDateTime"},
{0X00404009, "SQ", "HumanPerformerCodeSequence"},
{0X00404010, "DT", "ScheduledProcedureStepModificationDateTime"},
{0X00404011, "DT", "ExpectedCompletionDateTime"},
{0X00404015, "SQ", "ResultingGeneralPurposePerformedProcedureStepsSequence"},
{0X00404016, "SQ", "ReferencedGeneralPurposeScheduledProcedureStepSequence"},
{0X00404018, "SQ", "ScheduledWorkitemCodeSequence"},
{0X00404019, "SQ", "PerformedWorkitemCodeSequence"},
{0X00404020, "CS", "InputAvailabilityFlag"},
{0X00404021, "SQ", "InputInformationSequence"},
{0X00404022, "SQ", "RelevantInformationSequence"},
{0X00404023, "UI", "ReferencedGeneralPurposeScheduledProcedureStepTransactionUID"},
{0X00404025, "SQ", "ScheduledStationNameCodeSequence"},
{0X00404026, "SQ", "ScheduledStationClassCodeSequence"},
{0X00404027, "SQ", "ScheduledStationGeographicLocationCodeSequence"},
{0X00404028, "SQ", "PerformedStationNameCodeSequence"},
{0X00404029, "SQ", "PerformedStationClassCodeSequence"},
{0X00404030, "SQ", "PerformedStationGeographicLocationCodeSequence"},
{0X00404031, "SQ", "RequestedSubsequentWorkitemCodeSequence"},
{0X00404032, "SQ", "NonDICOMOutputCodeSequence"},
{0X00404033, "SQ", "OutputInformationSequence"},
{0X00404034, "SQ", "ScheduledHumanPerformersSequence"},
{0X00404035, "SQ", "ActualHumanPerformersSequence"},
{0X00404036, "LO", "HumanPerformerOrganization"},
{0X00404037, "PN", "HumanPerformerName"},
{0X00404040, "CS", "RawDataHandling"},
{0X00404041, "CS", "InputReadinessState"},
{0X00404050, "DT", "PerformedProcedureStepStartDateTime"},
{0X00404051, "DT", "PerformedProcedureStepEndDateTime"},
{0X00404052, "DT", "ProcedureStepCancellationDateTime"},
{0X00404070, "SQ", "OutputDestinationSequence"},
{0X00404071, "SQ", "DICOMStorageSequence"},
{0X00404072, "SQ", "STOWRSStorageSequence"},
{0X00404073, "UR", "StorageURL"},
{0X00404074, "SQ", "XDSStorageSequence"},
{0X00408302, "DS", "EntranceDoseInmGy"},
{0X00408303, "CS", "EntranceDoseDerivation"},
{0X00409092, "SQ", "ParametricMapFrameTypeSequence"},
{0X00409094, "SQ", "ReferencedImageRealWorldValueMappingSequence"},
{0X00409096, "SQ", "RealWorldValueMappingSequence"},
{0X00409098, "SQ", "PixelValueMappingCodeSequence"},
{0X00409210, "SH", "LUTLabel"},
{0X00409211, "US", "RealWorldValueLastValueMapped"},
{0X00409212, "FD", "RealWorldValueLUTData"},
{0X00409213, "FD", "DoubleFloatRealWorldValueLastValueMapped"},
{0X00409214, "FD", "DoubleFloatRealWorldValueFirstValueMapped"},
{0X00409216, "US", "RealWorldValueFirstValueMapped"},
{0X00409220, "SQ", "QuantityDefinitionSequence"},
{0X00409224, "FD", "RealWorldValueIntercept"},
{0X00409225, "FD", "RealWorldValueSlope"},
{0X0040A007, "CS", "FindingsFlagTrial"},
{0X0040A010, "CS", "RelationshipType"},
{0X0040A020, "SQ", "FindingsSequenceTrial"},
{0X0040A021, "UI", "FindingsGroupUIDTrial"},
{0X0040A022, "UI", "ReferencedFindingsGroupUIDTrial"},
{0X0040A023, "DA", "FindingsGroupRecordingDateTrial"},
{0X0040A024, "TM", "FindingsGroupRecordingTimeTrial"},
{0X0040A026, "SQ", "FindingsSourceCategoryCodeSequenceTrial"},
{0X0040A027, "LO", "VerifyingOrganization"},
{0X0040A028, "SQ", "DocumentingOrganizationIdentifierCodeSequenceTrial"},
{0X0040A030, "DT", "VerificationDateTime"},
{0X0040A032, "DT", "ObservationDateTime"},
{0X0040A033, "DT", "ObservationStartDateTime"},
{0X0040A040, "CS", "ValueType"},
{0X0040A043, "SQ", "ConceptNameCodeSequence"},
{0X0040A047, "LO", "MeasurementPrecisionDescriptionTrial"},
{0X0040A050, "CS", "ContinuityOfContent"},
{0X0040A057, "CS", "UrgencyOrPriorityAlertsTrial"},
{0X0040A060, "LO", "SequencingIndicatorTrial"},
{0X0040A066, "SQ", "DocumentIdentifierCodeSequenceTrial"},
{0X0040A067, "PN", "DocumentAuthorTrial"},
{0X0040A068, "SQ", "DocumentAuthorIdentifierCodeSequenceTrial"},
{0X0040A070, "SQ", "IdentifierCodeSequenceTrial"},
{0X0040A073, "SQ", "VerifyingObserverSequence"},
{0X0040A074, "OB", "ObjectBinaryIdentifierTrial"},
{0X0040A075, "PN", "VerifyingObserverName"},
{0X0040A076, "SQ", "DocumentingObserverIdentifierCodeSequenceTrial"},
{0X0040A078, "SQ", "AuthorObserverSequence"},
{0X0040A07A, "SQ", "ParticipantSequence"},
{0X0040A07C, "SQ", "CustodialOrganizationSequence"},
{0X0040A080, "CS", "ParticipationType"},
{0X0040A082, "DT", "ParticipationDateTime"},
{0X0040A084, "CS", "ObserverType"},
{0X0040A085, "SQ", "ProcedureIdentifierCodeSequenceTrial"},
{0X0040A088, "SQ", "VerifyingObserverIdentificationCodeSequence"},
{0X0040A089, "OB", "ObjectDirectoryBinaryIdentifierTrial"},
{0X0040A090, "SQ", "EquivalentCDADocumentSequence"},
{0X0040A0B0, "US", "ReferencedWaveformChannels"},
{0X0040A110, "DA", "DateOfDocumentOrVerbalTransactionTrial"},
{0X0040A112, "TM", "TimeOfDocumentCreationOrVerbalTransactionTrial"},
{0X0040A120, "DT", "DateTime"},
{0X0040A121, "DA", "Date"},
{0X0040A122, "TM", "Time"},
{0X0040A123, "PN", "PersonName"},
{0X0040A124, "UI", "UID"},
{0X0040A125, "CS", "ReportStatusIDTrial"},
{0X0040A130, "CS", "TemporalRangeType"},
{0X0040A132, "UL", "ReferencedSamplePositions"},
{0X0040A136, "US", "ReferencedFrameNumbers"},
{0X0040A138, "DS", "ReferencedTimeOffsets"},
{0X0040A13A, "DT", "ReferencedDateTime"},
{0X0040A160, "UT", "TextValue"},
{0X0040A161, "FD", "FloatingPointValue"},
{0X0040A162, "SL", "RationalNumeratorValue"},
{0X0040A163, "UL", "RationalDenominatorValue"},
{0X0040A167, "SQ", "ObservationCategoryCodeSequenceTrial"},
{0X0040A168, "SQ", "ConceptCodeSequence"},
{0X0040A16A, "ST", "BibliographicCitationTrial"},
{0X0040A170, "SQ", "PurposeOfReferenceCodeSequence"},
{0X0040A171, "UI", "ObservationUID"},
{0X0040A172, "UI", "ReferencedObservationUIDTrial"},
{0X0040A173, "CS", "ReferencedObservationClassTrial"},
{0X0040A174, "CS", "ReferencedObjectObservationClassTrial"},
{0X0040A180, "US", "AnnotationGroupNumber"},
{0X0040A192, "DA", "ObservationDateTrial"},
{0X0040A193, "TM", "ObservationTimeTrial"},
{0X0040A194, "CS", "MeasurementAutomationTrial"},
{0X0040A195, "SQ", "ModifierCodeSequence"},
{0X0040A224, "ST", "IdentificationDescriptionTrial"},
{0X0040A290, "CS", "CoordinatesSetGeometricTypeTrial"},
{0X0040A296, "SQ", "AlgorithmCodeSequenceTrial"},
{0X0040A297, "ST", "AlgorithmDescriptionTrial"},
{0X0040A29A, "SL", "PixelCoordinatesSetTrial"},
{0X0040A300, "SQ", "MeasuredValueSequence"},
{0X0040A301, "SQ", "NumericValueQualifierCodeSequence"},
{0X0040A307, "PN", "CurrentObserverTrial"},
{0X0040A30A, "DS", "NumericValue"},
{0X0040A313, "SQ", "ReferencedAccessionSequenceTrial"},
{0X0040A33A, "ST", "ReportStatusCommentTrial"},
{0X0040A340, "SQ", "ProcedureContextSequenceTrial"},
{0X0040A352, "PN", "VerbalSourceTrial"},
{0X0040A353, "ST", "AddressTrial"},
{0X0040A354, "LO", "TelephoneNumberTrial"},
{0X0040A358, "SQ", "VerbalSourceIdentifierCodeSequenceTrial"},
{0X0040A360, "SQ", "PredecessorDocumentsSequence"},
{0X0040A370, "SQ", "ReferencedRequestSequence"},
{0X0040A372, "SQ", "PerformedProcedureCodeSequence"},
{0X0040A375, "SQ", "CurrentRequestedProcedureEvidenceSequence"},
{0X0040A380, "SQ", "ReportDetailSequenceTrial"},
{0X0040A385, "SQ", "PertinentOtherEvidenceSequence"},
{0X0040A390, "SQ", "HL7StructuredDocumentReferenceSequence"},
{0X0040A402, "UI", "ObservationSubjectUIDTrial"},
{0X0040A403, "CS", "ObservationSubjectClassTrial"},
{0X0040A404, "SQ", "ObservationSubjectTypeCodeSequenceTrial"},
{0X0040A491, "CS", "CompletionFlag"},
{0X0040A492, "LO", "CompletionFlagDescription"},
{0X0040A493, "CS", "VerificationFlag"},
{0X0040A494, "CS", "ArchiveRequested"},
{0X0040A496, "CS", "PreliminaryFlag"},
{0X0040A504, "SQ", "ContentTemplateSequence"},
{0X0040A525, "SQ", "IdenticalDocumentsSequence"},
{0X0040A600, "CS", "ObservationSubjectContextFlagTrial"},
{0X0040A601, "CS", "ObserverContextFlagTrial"},
{0X0040A603, "CS", "ProcedureContextFlagTrial"},
{0X0040A730, "SQ", "ContentSequence"},
{0X0040A731, "SQ", "RelationshipSequenceTrial"},
{0X0040A732, "SQ", "RelationshipTypeCodeSequenceTrial"},
{0X0040A744, "SQ", "LanguageCodeSequenceTrial"},
{0X0040A801, "SQ", "TabulatedValuesSequence"},
{0X0040A802, "UL", "NumberOfTableRows"},
{0X0040A803, "UL", "NumbeOfTableColumns"},
{0X0040A804, "UL", "TableRowNumber"},
{0X0040A805, "UL", "TableColumnNumber"},
{0X0040A806, "SQ", "TableRowDefinitionSequence"},
{0X0040A807, "SQ", "TableColumnDefinitionSequence"},
{0X0040A808, "SQ", "CellValuesSequence"},
{0X0040A992, "ST", "UniformResourceLocatorTrial"},
{0X0040B020, "SQ", "WaveformAnnotationSequence"},
{0X0040DB00, "CS", "TemplateIdentifier"},
{0X0040DB06, "DT", "TemplateVersion"},
{0X0040DB07, "DT", "TemplateLocalVersion"},
{0X0040DB0B, "CS", "TemplateExtensionFlag"},
{0X0040DB0C, "UI", "TemplateExtensionOrganizationUID"},
{0X0040DB0D, "UI", "TemplateExtensionCreatorUID"},
{0X0040DB73, "UL", "ReferencedContentItemIdentifier"},
{0X0040E001, "ST", "HL7InstanceIdentifier"},
{0X0040E004, "DT", "HL7DocumentEffectiveTime"},
{0X0040E006, "SQ", "HL7DocumentTypeCodeSequence"},
{0X0040E008, "SQ", "DocumentClassCodeSequence"},
{0X0040E010, "UR", "RetrieveURI"},
{0X0040E011, "UI", "RetrieveLocationUID"},
{0X0040E020, "CS", "TypeOfInstances"},
{0X0040E021, "SQ", "DICOMRetrievalSequence"},
{0X0040E022, "SQ", "DICOMMediaRetrievalSequence"},
{0X0040E023, "SQ", "WADORetrievalSequence"},
{0X0040E024, "SQ", "XDSRetrievalSequence"},
{0X0040E025, "SQ", "WADORSRetrievalSequence"},
{0X0040E030, "UI", "RepositoryUniqueID"},
{0X0040E031, "UI", "HomeCommunityID"},
{0X00420010, "ST", "DocumentTitle"},
{0X00420011, "OB", "EncapsulatedDocument"},
{0X00420012, "LO", "MIMETypeOfEncapsulatedDocument"},
{0X00420013, "SQ", "SourceInstanceSequence"},
{0X00420014, "LO", "ListOfMIMETypes"},
{0X00420015, "UL", "EncapsulatedDocumentLength"},
{0X00440001, "ST", "ProductPackageIdentifier"},
{0X00440002, "CS", "SubstanceAdministrationApproval"},
{0X00440003, "LT", "ApprovalStatusFurtherDescription"},
{0X00440004, "DT", "ApprovalStatusDateTime"},
{0X00440007, "SQ", "ProductTypeCodeSequence"},
{0X00440008, "LO", "ProductName"},
{0X00440009, "LT", "ProductDescription"},
{0X0044000A, "LO", "ProductLotIdentifier"},
{0X0044000B, "DT", "ProductExpirationDateTime"},
{0X00440010, "DT", "SubstanceAdministrationDateTime"},
{0X00440011, "LO", "SubstanceAdministrationNotes"},
{0X00440012, "LO", "SubstanceAdministrationDeviceID"},
{0X00440013, "SQ", "ProductParameterSequence"},
{0X00440019, "SQ", "SubstanceAdministrationParameterSequence"},
{0X00440100, "SQ", "ApprovalSequence"},
{0X00440101, "SQ", "AssertionCodeSequence"},
{0X00440102, "UI", "AssertionUID"},
{0X00440103, "SQ", "AsserterIdentificationSequence"},
{0X00440104, "DT", "AssertionDateTime"},
{0X00440105, "DT", "AssertionExpirationDateTime"},
{0X00440106, "UT", "AssertionComments"},
{0X00440107, "SQ", "RelatedAssertionSequence"},
{0X00440108, "UI", "ReferencedAssertionUID"},
{0X00440109, "SQ", "ApprovalSubjectSequence"},
{0X0044010A, "SQ", "OrganizationalRoleCodeSequence"},
{0X00460012, "LO", "LensDescription"},
{0X00460014, "SQ", "RightLensSequence"},
{0X00460015, "SQ", "LeftLensSequence"},
{0X00460016, "SQ", "UnspecifiedLateralityLensSequence"},
{0X00460018, "SQ", "CylinderSequence"},
{0X00460028, "SQ", "PrismSequence"},
{0X00460030, "FD", "HorizontalPrismPower"},
{0X00460032, "CS", "HorizontalPrismBase"},
{0X00460034, "FD", "VerticalPrismPower"},
{0X00460036, "CS", "VerticalPrismBase"},
{0X00460038, "CS", "LensSegmentType"},
{0X00460040, "FD", "OpticalTransmittance"},
{0X00460042, "FD", "ChannelWidth"},
{0X00460044, "FD", "PupilSize"},
{0X00460046, "FD", "CornealSize"},
{0X00460047, "SQ", "CornealSizeSequence"},
{0X00460050, "SQ", "AutorefractionRightEyeSequence"},
{0X00460052, "SQ", "AutorefractionLeftEyeSequence"},
{0X00460060, "FD", "DistancePupillaryDistance"},
{0X00460062, "FD", "NearPupillaryDistance"},
{0X00460063, "FD", "IntermediatePupillaryDistance"},
{0X00460064, "FD", "OtherPupillaryDistance"},
{0X00460070, "SQ", "KeratometryRightEyeSequence"},
{0X00460071, "SQ", "KeratometryLeftEyeSequence"},
{0X00460074, "SQ", "SteepKeratometricAxisSequence"},
{0X00460075, "FD", "RadiusOfCurvature"},
{0X00460076, "FD", "KeratometricPower"},
{0X00460077, "FD", "KeratometricAxis"},
{0X00460080, "SQ", "FlatKeratometricAxisSequence"},
{0X00460092, "CS", "BackgroundColor"},
{0X00460094, "CS", "Optotype"},
{0X00460095, "CS", "OptotypePresentation"},
{0X00460097, "SQ", "SubjectiveRefractionRightEyeSequence"},
{0X00460098, "SQ", "SubjectiveRefractionLeftEyeSequence"},
{0X00460100, "SQ", "AddNearSequence"},
{0X00460101, "SQ", "AddIntermediateSequence"},
{0X00460102, "SQ", "AddOtherSequence"},
{0X00460104, "FD", "AddPower"},
{0X00460106, "FD", "ViewingDistance"},
{0X00460110, "SQ", "CorneaMeasurementsSequence"},
{0X00460111, "SQ", "SourceOfCorneaMeasurementDataCodeSequence"},
{0X00460112, "SQ", "SteepCornealAxisSequence"},
{0X00460113, "SQ", "FlatCornealAxisSequence"},
{0X00460114, "FD", "CornealPower"},
{0X00460115, "FD", "CornealAxis"},
{0X00460116, "SQ", "CorneaMeasurementMethodCodeSequence"},
{0X00460117, "FL", "RefractiveIndexOfCornea"},
{0X00460118, "FL", "RefractiveIndexOfAqueousHumor"},
{0X00460121, "SQ", "VisualAcuityTypeCodeSequence"},
{0X00460122, "SQ", "VisualAcuityRightEyeSequence"},
{0X00460123, "SQ", "VisualAcuityLeftEyeSequence"},
{0X00460124, "SQ", "VisualAcuityBothEyesOpenSequence"},
{0X00460125, "CS", "ViewingDistanceType"},
{0X00460135, "SS", "VisualAcuityModifiers"},
{0X00460137, "FD", "DecimalVisualAcuity"},
{0X00460139, "LO", "OptotypeDetailedDefinition"},
{0X00460145, "SQ", "ReferencedRefractiveMeasurementsSequence"},
{0X00460146, "FD", "SpherePower"},
{0X00460147, "FD", "CylinderPower"},
{0X00460201, "CS", "CornealTopographySurface"},
{0X00460202, "FL", "CornealVertexLocation"},
{0X00460203, "FL", "PupilCentroidXCoordinate"},
{0X00460204, "FL", "PupilCentroidYCoordinate"},
{0X00460205, "FL", "EquivalentPupilRadius"},
{0X00460207, "SQ", "CornealTopographyMapTypeCodeSequence"},
{0X00460208, "IS", "VerticesOfTheOutlineOfPupil"},
{0X00460210, "SQ", "CornealTopographyMappingNormalsSequence"},
{0X00460211, "SQ", "MaximumCornealCurvatureSequence"},
{0X00460212, "FL", "MaximumCornealCurvature"},
{0X00460213, "FL", "MaximumCornealCurvatureLocation"},
{0X00460215, "SQ", "MinimumKeratometricSequence"},
{0X00460218, "SQ", "SimulatedKeratometricCylinderSequence"},
{0X00460220, "FL", "AverageCornealPower"},
{0X00460224, "FL", "CornealISValue"},
{0X00460227, "FL", "AnalyzedArea"},
{0X00460230, "FL", "SurfaceRegularityIndex"},
{0X00460232, "FL", "SurfaceAsymmetryIndex"},
{0X00460234, "FL", "CornealEccentricityIndex"},
{0X00460236, "FL", "KeratoconusPredictionIndex"},
{0X00460238, "FL", "DecimalPotentialVisualAcuity"},
{0X00460242, "CS", "CornealTopographyMapQualityEvaluation"},
{0X00460244, "SQ", "SourceImageCornealProcessedDataSequence"},
{0X00460247, "FL", "CornealPointLocation"},
{0X00460248, "CS", "CornealPointEstimated"},
{0X00460249, "FL", "AxialPower"},
{0X00460250, "FL", "TangentialPower"},
{0X00460251, "FL", "RefractivePower"},
{0X00460252, "FL", "RelativeElevation"},
{0X00460253, "FL", "CornealWavefront"},
{0X00480001, "FL", "ImagedVolumeWidth"},
{0X00480002, "FL", "ImagedVolumeHeight"},
{0X00480003, "FL", "ImagedVolumeDepth"},
{0X00480006, "UL", "TotalPixelMatrixColumns"},
{0X00480007, "UL", "TotalPixelMatrixRows"},
{0X00480008, "SQ", "TotalPixelMatrixOriginSequence"},
{0X00480010, "CS", "SpecimenLabelInImage"},
{0X00480011, "CS", "FocusMethod"},
{0X00480012, "CS", "ExtendedDepthOfField"},
{0X00480013, "US", "NumberOfFocalPlanes"},
{0X00480014, "FL", "DistanceBetweenFocalPlanes"},
{0X00480015, "US", "RecommendedAbsentPixelCIELabValue"},
{0X00480100, "SQ", "IlluminatorTypeCodeSequence"},
{0X00480102, "DS", "ImageOrientationSlide"},
{0X00480105, "SQ", "OpticalPathSequence"},
{0X00480106, "SH", "OpticalPathIdentifier"},
{0X00480107, "ST", "OpticalPathDescription"},
{0X00480108, "SQ", "IlluminationColorCodeSequence"},
{0X00480110, "SQ", "SpecimenReferenceSequence"},
{0X00480111, "DS", "CondenserLensPower"},
{0X00480112, "DS", "ObjectiveLensPower"},
{0X00480113, "DS", "ObjectiveLensNumericalAperture"},
{0X00480120, "SQ", "PaletteColorLookupTableSequence"},
{0X00480200, "SQ", "ReferencedImageNavigationSequence"},
{0X00480201, "US", "TopLeftHandCornerOfLocalizerArea"},
{0X00480202, "US", "BottomRightHandCornerOfLocalizerArea"},
{0X00480207, "SQ", "OpticalPathIdentificationSequence"},
{0X0048021A, "SQ", "PlanePositionSlideSequence"},
{0X0048021E, "SL", "ColumnPositionInTotalImagePixelMatrix"},
{0X0048021F, "SL", "RowPositionInTotalImagePixelMatrix"},
{0X00480301, "CS", "PixelOriginInterpretation"},
{0X00480302, "UL", "NumberOfOpticalPaths"},
{0X00480303, "UL", "TotalPixelMatrixFocalPlanes"},
{0X00500004, "CS", "CalibrationImage"},
{0X00500010, "SQ", "DeviceSequence"},
{0X00500012, "SQ", "ContainerComponentTypeCodeSequence"},
{0X00500013, "FD", "ContainerComponentThickness"},
{0X00500014, "DS", "DeviceLength"},
{0X00500015, "FD", "ContainerComponentWidth"},
{0X00500016, "DS", "DeviceDiameter"},
{0X00500017, "CS", "DeviceDiameterUnits"},
{0X00500018, "DS", "DeviceVolume"},
{0X00500019, "DS", "InterMarkerDistance"},
{0X0050001A, "CS", "ContainerComponentMaterial"},
{0X0050001B, "LO", "ContainerComponentID"},
{0X0050001C, "FD", "ContainerComponentLength"},
{0X0050001D, "FD", "ContainerComponentDiameter"},
{0X0050001E, "LO", "ContainerComponentDescription"},
{0X00500020, "LO", "DeviceDescription"},
{0X00500021, "ST", "LongDeviceDescription"},
{0X00520001, "FL", "ContrastBolusIngredientPercentByVolume"},
{0X00520002, "FD", "OCTFocalDistance"},
{0X00520003, "FD", "BeamSpotSize"},
{0X00520004, "FD", "EffectiveRefractiveIndex"},
{0X00520006, "CS", "OCTAcquisitionDomain"},
{0X00520007, "FD", "OCTOpticalCenterWavelength"},
{0X00520008, "FD", "AxialResolution"},
{0X00520009, "FD", "RangingDepth"},
{0X00520011, "FD", "ALineRate"},
{0X00520012, "US", "ALinesPerFrame"},
{0X00520013, "FD", "CatheterRotationalRate"},
{0X00520014, "FD", "ALinePixelSpacing"},
{0X00520016, "SQ", "ModeOfPercutaneousAccessSequence"},
{0X00520025, "SQ", "IntravascularOCTFrameTypeSequence"},
{0X00520026, "CS", "OCTZOffsetApplied"},
{0X00520027, "SQ", "IntravascularFrameContentSequence"},
{0X00520028, "FD", "IntravascularLongitudinalDistance"},
{0X00520029, "SQ", "IntravascularOCTFrameContentSequence"},
{0X00520030, "SS", "OCTZOffsetCorrection"},
{0X00520031, "CS", "CatheterDirectionOfRotation"},
{0X00520033, "FD", "SeamLineLocation"},
{0X00520034, "FD", "FirstALineLocation"},
{0X00520036, "US", "SeamLineIndex"},
{0X00520038, "US", "NumberOfPaddedALines"},
{0X00520039, "CS", "InterpolationType"},
{0X0052003A, "CS", "RefractiveIndexApplied"},
{0X00540010, "US", "EnergyWindowVector"},
{0X00540011, "US", "NumberOfEnergyWindows"},
{0X00540012, "SQ", "EnergyWindowInformationSequence"},
{0X00540013, "SQ", "EnergyWindowRangeSequence"},
{0X00540014, "DS", "EnergyWindowLowerLimit"},
{0X00540015, "DS", "EnergyWindowUpperLimit"},
{0X00540016, "SQ", "RadiopharmaceuticalInformationSequence"},
{0X00540017, "IS", "ResidualSyringeCounts"},
{0X00540018, "SH", "EnergyWindowName"},
{0X00540020, "US", "DetectorVector"},
{0X00540021, "US", "NumberOfDetectors"},
{0X00540022, "SQ", "DetectorInformationSequence"},
{0X00540030, "US", "PhaseVector"},
{0X00540031, "US", "NumberOfPhases"},
{0X00540032, "SQ", "PhaseInformationSequence"},
{0X00540033, "US", "NumberOfFramesInPhase"},
{0X00540036, "IS", "PhaseDelay"},
{0X00540038, "IS", "PauseBetweenFrames"},
{0X00540039, "CS", "PhaseDescription"},
{0X00540050, "US", "RotationVector"},
{0X00540051, "US", "NumberOfRotations"},
{0X00540052, "SQ", "RotationInformationSequence"},
{0X00540053, "US", "NumberOfFramesInRotation"},
{0X00540060, "US", "RRIntervalVector"},
{0X00540061, "US", "NumberOfRRIntervals"},
{0X00540062, "SQ", "GatedInformationSequence"},
{0X00540063, "SQ", "DataInformationSequence"},
{0X00540070, "US", "TimeSlotVector"},
{0X00540071, "US", "NumberOfTimeSlots"},
{0X00540072, "SQ", "TimeSlotInformationSequence"},
{0X00540073, "DS", "TimeSlotTime"},
{0X00540080, "US", "SliceVector"},
{0X00540081, "US", "NumberOfSlices"},
{0X00540090, "US", "AngularViewVector"},
{0X00540100, "US", "TimeSliceVector"},
{0X00540101, "US", "NumberOfTimeSlices"},
{0X00540200, "DS", "StartAngle"},
{0X00540202, "CS", "TypeOfDetectorMotion"},
{0X00540210, "IS", "TriggerVector"},
{0X00540211, "US", "NumberOfTriggersInPhase"},
{0X00540220, "SQ", "ViewCodeSequence"},
{0X00540222, "SQ", "ViewModifierCodeSequence"},
{0X00540300, "SQ", "RadionuclideCodeSequence"},
{0X00540302, "SQ", "AdministrationRouteCodeSequence"},
{0X00540304, "SQ", "RadiopharmaceuticalCodeSequence"},
{0X00540306, "SQ", "CalibrationDataSequence"},
{0X00540308, "US", "EnergyWindowNumber"},
{0X00540400, "SH", "ImageID"},
{0X00540410, "SQ", "PatientOrientationCodeSequence"},
{0X00540412, "SQ", "PatientOrientationModifierCodeSequence"},
{0X00540414, "SQ", "PatientGantryRelationshipCodeSequence"},
{0X00540500, "CS", "SliceProgressionDirection"},
{0X00540501, "CS", "ScanProgressionDirection"},
{0X00541000, "CS", "SeriesType"},
{0X00541001, "CS", "Units"},
{0X00541002, "CS", "CountsSource"},
{0X00541004, "CS", "ReprojectionMethod"},
{0X00541006, "CS", "SUVType"},
{0X00541100, "CS", "RandomsCorrectionMethod"},
{0X00541101, "LO", "AttenuationCorrectionMethod"},
{0X00541102, "CS", "DecayCorrection"},
{0X00541103, "LO", "ReconstructionMethod"},
{0X00541104, "LO", "DetectorLinesOfResponseUsed"},
{0X00541105, "LO", "ScatterCorrectionMethod"},
{0X00541200, "DS", "AxialAcceptance"},
{0X00541201, "IS", "AxialMash"},
{0X00541202, "IS", "TransverseMash"},
{0X00541203, "DS", "DetectorElementSize"},
{0X00541210, "DS", "CoincidenceWindowWidth"},
{0X00541220, "CS", "SecondaryCountsType"},
{0X00541300, "DS", "FrameReferenceTime"},
{0X00541310, "IS", "PrimaryPromptsCountsAccumulated"},
{0X00541311, "IS", "SecondaryCountsAccumulated"},
{0X00541320, "DS", "SliceSensitivityFactor"},
{0X00541321, "DS", "DecayFactor"},
{0X00541322, "DS", "DoseCalibrationFactor"},
{0X00541323, "DS", "ScatterFractionFactor"},
{0X00541324, "DS", "DeadTimeFactor"},
{0X00541330, "US", "ImageIndex"},
{0X00541400, "CS", "CountsIncluded"},
{0X00541401, "CS", "DeadTimeCorrectionFlag"},
{0X00603000, "SQ", "HistogramSequence"},
{0X00603002, "US", "HistogramNumberOfBins"},
{0X00603004, "US", "HistogramFirstBinValue"},
{0X00603006, "US", "HistogramLastBinValue"},
{0X00603008, "US", "HistogramBinWidth"},
{0X00603010, "LO", "HistogramExplanation"},
{0X00603020, "UL", "HistogramData"},
{0X00620001, "CS", "SegmentationType"},
{0X00620002, "SQ", "SegmentSequence"},
{0X00620003, "SQ", "SegmentedPropertyCategoryCodeSequence"},
{0X00620004, "US", "SegmentNumber"},
{0X00620005, "LO", "SegmentLabel"},
{0X00620006, "ST", "SegmentDescription"},
{0X00620007, "SQ", "SegmentationAlgorithmIdentificationSequence"},
{0X00620008, "CS", "SegmentAlgorithmType"},
{0X00620009, "LO", "SegmentAlgorithmName"},
{0X0062000A, "SQ", "SegmentIdentificationSequence"},
{0X0062000B, "US", "ReferencedSegmentNumber"},
{0X0062000C, "US", "RecommendedDisplayGrayscaleValue"},
{0X0062000D, "US", "RecommendedDisplayCIELabValue"},
{0X0062000E, "US", "MaximumFractionalValue"},
{0X0062000F, "SQ", "SegmentedPropertyTypeCodeSequence"},
{0X00620010, "CS", "SegmentationFractionalType"},
{0X00620011, "SQ", "SegmentedPropertyTypeModifierCodeSequence"},
{0X00620012, "SQ", "UsedSegmentsSequence"},
{0X00620013, "CS", "SegmentsOverlap"},
{0X00620020, "UT", "TrackingID"},
{0X00620021, "UI", "TrackingUID"},
{0X00640002, "SQ", "DeformableRegistrationSequence"},
{0X00640003, "UI", "SourceFrameOfReferenceUID"},
{0X00640005, "SQ", "DeformableRegistrationGridSequence"},
{0X00640007, "UL", "GridDimensions"},
{0X00640008, "FD", "GridResolution"},
{0X00640009, "OF", "VectorGridData"},
{0X0064000F, "SQ", "PreDeformationMatrixRegistrationSequence"},
{0X00640010, "SQ", "PostDeformationMatrixRegistrationSequence"},
{0X00660001, "UL", "NumberOfSurfaces"},
{0X00660002, "SQ", "SurfaceSequence"},
{0X00660003, "UL", "SurfaceNumber"},
{0X00660004, "LT", "SurfaceComments"},
{0X00660009, "CS", "SurfaceProcessing"},
{0X0066000A, "FL", "SurfaceProcessingRatio"},
{0X0066000B, "LO", "SurfaceProcessingDescription"},
{0X0066000C, "FL", "RecommendedPresentationOpacity"},
{0X0066000D, "CS", "RecommendedPresentationType"},
{0X0066000E, "CS", "FiniteVolume"},
{0X00660010, "CS", "Manifold"},
{0X00660011, "SQ", "SurfacePointsSequence"},
{0X00660012, "SQ", "SurfacePointsNormalsSequence"},
{0X00660013, "SQ", "SurfaceMeshPrimitivesSequence"},
{0X00660015, "UL", "NumberOfSurfacePoints"},
{0X00660016, "OF", "PointCoordinatesData"},
{0X00660017, "FL", "PointPositionAccuracy"},
{0X00660018, "FL", "MeanPointDistance"},
{0X00660019, "FL", "MaximumPointDistance"},
{0X0066001A, "FL", "PointsBoundingBoxCoordinates"},
{0X0066001B, "FL", "AxisOfRotation"},
{0X0066001C, "FL", "CenterOfRotation"},
{0X0066001E, "UL", "NumberOfVectors"},
{0X0066001F, "US", "VectorDimensionality"},
{0X00660020, "FL", "VectorAccuracy"},
{0X00660021, "OF", "VectorCoordinateData"},
{0X00660022, "OD", "DoublePointCoordinatesData"},
{0X00660023, "OW", "TrianglePointIndexList"},
{0X00660024, "OW", "EdgePointIndexList"},
{0X00660025, "OW", "VertexPointIndexList"},
{0X00660026, "SQ", "TriangleStripSequence"},
{0X00660027, "SQ", "TriangleFanSequence"},
{0X00660028, "SQ", "LineSequence"},
{0X00660029, "OW", "PrimitivePointIndexList"},
{0X0066002A, "UL", "SurfaceCount"},
{0X0066002B, "SQ", "ReferencedSurfaceSequence"},
{0X0066002C, "UL", "ReferencedSurfaceNumber"},
{0X0066002D, "SQ", "SegmentSurfaceGenerationAlgorithmIdentificationSequence"},
{0X0066002E, "SQ", "SegmentSurfaceSourceInstanceSequence"},
{0X0066002F, "SQ", "AlgorithmFamilyCodeSequence"},
{0X00660030, "SQ", "AlgorithmNameCodeSequence"},
{0X00660031, "LO", "AlgorithmVersion"},
{0X00660032, "LT", "AlgorithmParameters"},
{0X00660034, "SQ", "FacetSequence"},
{0X00660035, "SQ", "SurfaceProcessingAlgorithmIdentificationSequence"},
{0X00660036, "LO", "AlgorithmName"},
{0X00660037, "FL", "RecommendedPointRadius"},
{0X00660038, "FL", "RecommendedLineThickness"},
{0X00660040, "OL", "LongPrimitivePointIndexList"},
{0X00660041, "OL", "LongTrianglePointIndexList"},
{0X00660042, "OL", "LongEdgePointIndexList"},
{0X00660043, "OL", "LongVertexPointIndexList"},
{0X00660101, "SQ", "TrackSetSequence"},
{0X00660102, "SQ", "TrackSequence"},
{0X00660103, "OW", "RecommendedDisplayCIELabValueList"},
{0X00660104, "SQ", "TrackingAlgorithmIdentificationSequence"},
{0X00660105, "UL", "TrackSetNumber"},
{0X00660106, "LO", "TrackSetLabel"},
{0X00660107, "UT", "TrackSetDescription"},
{0X00660108, "SQ", "TrackSetAnatomicalTypeCodeSequence"},
{0X00660121, "SQ", "MeasurementsSequence"},
{0X00660124, "SQ", "TrackSetStatisticsSequence"},
{0X00660125, "OF", "FloatingPointValues"},
{0X00660129, "OL", "TrackPointIndexList"},
{0X00660130, "SQ", "TrackStatisticsSequence"},
{0X00660132, "SQ", "MeasurementValuesSequence"},
{0X00660133, "SQ", "DiffusionAcquisitionCodeSequence"},
{0X00660134, "SQ", "DiffusionModelCodeSequence"},
{0X00686210, "LO", "ImplantSize"},
{0X00686221, "LO", "ImplantTemplateVersion"},
{0X00686222, "SQ", "ReplacedImplantTemplateSequence"},
{0X00686223, "CS", "ImplantType"},
{0X00686224, "SQ", "DerivationImplantTemplateSequence"},
{0X00686225, "SQ", "OriginalImplantTemplateSequence"},
{0X00686226, "DT", "EffectiveDateTime"},
{0X00686230, "SQ", "ImplantTargetAnatomySequence"},
{0X00686260, "SQ", "InformationFromManufacturerSequence"},
{0X00686265, "SQ", "NotificationFromManufacturerSequence"},
{0X00686270, "DT", "InformationIssueDateTime"},
{0X00686280, "ST", "InformationSummary"},
{0X006862A0, "SQ", "ImplantRegulatoryDisapprovalCodeSequence"},
{0X006862A5, "FD", "OverallTemplateSpatialTolerance"},
{0X006862C0, "SQ", "HPGLDocumentSequence"},
{0X006862D0, "US", "HPGLDocumentID"},
{0X006862D5, "LO", "HPGLDocumentLabel"},
{0X006862E0, "SQ", "ViewOrientationCodeSequence"},
{0X006862F0, "SQ", "ViewOrientationModifierCodeSequence"},
{0X006862F2, "FD", "HPGLDocumentScaling"},
{0X00686300, "OB", "HPGLDocument"},
{0X00686310, "US", "HPGLContourPenNumber"},
{0X00686320, "SQ", "HPGLPenSequence"},
{0X00686330, "US", "HPGLPenNumber"},
{0X00686340, "LO", "HPGLPenLabel"},
{0X00686345, "ST", "HPGLPenDescription"},
{0X00686346, "FD", "RecommendedRotationPoint"},
{0X00686347, "FD", "BoundingRectangle"},
{0X00686350, "US", "ImplantTemplate3DModelSurfaceNumber"},
{0X00686360, "SQ", "SurfaceModelDescriptionSequence"},
{0X00686380, "LO", "SurfaceModelLabel"},
{0X00686390, "FD", "SurfaceModelScalingFactor"},
{0X006863A0, "SQ", "MaterialsCodeSequence"},
{0X006863A4, "SQ", "CoatingMaterialsCodeSequence"},
{0X006863A8, "SQ", "ImplantTypeCodeSequence"},
{0X006863AC, "SQ", "FixationMethodCodeSequence"},
{0X006863B0, "SQ", "MatingFeatureSetsSequence"},
{0X006863C0, "US", "MatingFeatureSetID"},
{0X006863D0, "LO", "MatingFeatureSetLabel"},
{0X006863E0, "SQ", "MatingFeatureSequence"},
{0X006863F0, "US", "MatingFeatureID"},
{0X00686400, "SQ", "MatingFeatureDegreeOfFreedomSequence"},
{0X00686410, "US", "DegreeOfFreedomID"},
{0X00686420, "CS", "DegreeOfFreedomType"},
{0X00686430, "SQ", "TwoDMatingFeatureCoordinatesSequence"},
{0X00686440, "US", "ReferencedHPGLDocumentID"},
{0X00686450, "FD", "TwoDMatingPoint"},
{0X00686460, "FD", "TwoDMatingAxes"},
{0X00686470, "SQ", "TwoDDegreeOfFreedomSequence"},
{0X00686490, "FD", "ThreeDDegreeOfFreedomAxis"},
{0X006864A0, "FD", "RangeOfFreedom"},
{0X006864C0, "FD", "ThreeDMatingPoint"},
{0X006864D0, "FD", "ThreeDMatingAxes"},
{0X006864F0, "FD", "TwoDDegreeOfFreedomAxis"},
{0X00686500, "SQ", "PlanningLandmarkPointSequence"},
{0X00686510, "SQ", "PlanningLandmarkLineSequence"},
{0X00686520, "SQ", "PlanningLandmarkPlaneSequence"},
{0X00686530, "US", "PlanningLandmarkID"},
{0X00686540, "LO", "PlanningLandmarkDescription"},
{0X00686545, "SQ", "PlanningLandmarkIdentificationCodeSequence"},
{0X00686550, "SQ", "TwoDPointCoordinatesSequence"},
{0X00686560, "FD", "TwoDPointCoordinates"},
{0X00686590, "FD", "ThreeDPointCoordinates"},
{0X006865A0, "SQ", "TwoDLineCoordinatesSequence"},
{0X006865B0, "FD", "TwoDLineCoordinates"},
{0X006865D0, "FD", "ThreeDLineCoordinates"},
{0X006865E0, "SQ", "TwoDPlaneCoordinatesSequence"},
{0X006865F0, "FD", "TwoDPlaneIntersection"},
{0X00686610, "FD", "ThreeDPlaneOrigin"},
{0X00686620, "FD", "ThreeDPlaneNormal"},
{0X00687001, "CS", "ModelModification"},
{0X00687002, "CS", "ModelMirroring"},
{0X00687003, "SQ", "ModelUsageCodeSequence"},
{0X00687004, "UI", "ModelGroupUID"},
{0X00687005, "UR", "RelativeURIReferenceWithinEncapsulatedDocument"},
{0X006A0001, "CS", "AnnotationCoordinateType"},
{0X006A0002, "SQ", "AnnotationGroupSequence"},
{0X006A0003, "UI", "AnnotationGroupUID"},
{0X006A0005, "LO", "AnnotationGroupLabel"},
{0X006A0006, "UT", "AnnotationGroupDescription"},
{0X006A0007, "CS", "AnnotationGroupGenerationType"},
{0X006A0008, "SQ", "AnnotationGroupAlgorithmIdentificationSequence"},
{0X006A0009, "SQ", "AnnotationPropertyCategoryCodeSequence"},
{0X006A000A, "SQ", "AnnotationPropertyTypeCodeSequence"},
{0X006A000B, "SQ", "AnnotationPropertyTypeModifierCodeSequence"},
{0X006A000C, "UL", "NumberOfAnnotations"},
{0X006A000D, "CS", "AnnotationAppliesToAllOpticalPaths"},
{0X006A000E, "SH", "ReferencedOpticalPathIdentifier"},
{0X006A000F, "CS", "AnnotationAppliesToAllZPlanes"},
{0X006A0010, "FD", "CommonZCoordinateValue"},
{0X006A0011, "OL", "AnnotationIndexList"},
{0X00700001, "SQ", "GraphicAnnotationSequence"},
{0X00700002, "CS", "GraphicLayer"},
{0X00700003, "CS", "BoundingBoxAnnotationUnits"},
{0X00700004, "CS", "AnchorPointAnnotationUnits"},
{0X00700005, "CS", "GraphicAnnotationUnits"},
{0X00700006, "ST", "UnformattedTextValue"},
{0X00700008, "SQ", "TextObjectSequence"},
{0X00700009, "SQ", "GraphicObjectSequence"},
{0X00700010, "FL", "BoundingBoxTopLeftHandCorner"},
{0X00700011, "FL", "BoundingBoxBottomRightHandCorner"},
{0X00700012, "CS", "BoundingBoxTextHorizontalJustification"},
{0X00700014, "FL", "AnchorPoint"},
{0X00700015, "CS", "AnchorPointVisibility"},
{0X00700020, "US", "GraphicDimensions"},
{0X00700021, "US", "NumberOfGraphicPoints"},
{0X00700022, "FL", "GraphicData"},
{0X00700023, "CS", "GraphicType"},
{0X00700024, "CS", "GraphicFilled"},
{0X00700040, "IS", "ImageRotationRetired"},
{0X00700041, "CS", "ImageHorizontalFlip"},
{0X00700042, "US", "ImageRotation"},
{0X00700050, "US", "DisplayedAreaTopLeftHandCornerTrial"},
{0X00700051, "US", "DisplayedAreaBottomRightHandCornerTrial"},
{0X00700052, "SL", "DisplayedAreaTopLeftHandCorner"},
{0X00700053, "SL", "DisplayedAreaBottomRightHandCorner"},
{0X0070005A, "SQ", "DisplayedAreaSelectionSequence"},
{0X00700060, "SQ", "GraphicLayerSequence"},
{0X00700062, "IS", "GraphicLayerOrder"},
{0X00700066, "US", "GraphicLayerRecommendedDisplayGrayscaleValue"},
{0X00700067, "US", "GraphicLayerRecommendedDisplayRGBValue"},
{0X00700068, "LO", "GraphicLayerDescription"},
{0X00700080, "CS", "ContentLabel"},
{0X00700081, "LO", "ContentDescription"},
{0X00700082, "DA", "PresentationCreationDate"},
{0X00700083, "TM", "PresentationCreationTime"},
{0X00700084, "PN", "ContentCreatorName"},
{0X00700086, "SQ", "ContentCreatorIdentificationCodeSequence"},
{0X00700087, "SQ", "AlternateContentDescriptionSequence"},
{0X00700100, "CS", "PresentationSizeMode"},
{0X00700101, "DS", "PresentationPixelSpacing"},
{0X00700102, "IS", "PresentationPixelAspectRatio"},
{0X00700103, "FL", "PresentationPixelMagnificationRatio"},
{0X00700207, "LO", "GraphicGroupLabel"},
{0X00700208, "ST", "GraphicGroupDescription"},
{0X00700209, "SQ", "CompoundGraphicSequence"},
{0X00700226, "UL", "CompoundGraphicInstanceID"},
{0X00700227, "LO", "FontName"},
{0X00700228, "CS", "FontNameType"},
{0X00700229, "LO", "CSSFontName"},
{0X00700230, "FD", "RotationAngle"},
{0X00700231, "SQ", "TextStyleSequence"},
{0X00700232, "SQ", "LineStyleSequence"},
{0X00700233, "SQ", "FillStyleSequence"},
{0X00700234, "SQ", "GraphicGroupSequence"},
{0X00700241, "US", "TextColorCIELabValue"},
{0X00700242, "CS", "HorizontalAlignment"},
{0X00700243, "CS", "VerticalAlignment"},
{0X00700244, "CS", "ShadowStyle"},
{0X00700245, "FL", "ShadowOffsetX"},
{0X00700246, "FL", "ShadowOffsetY"},
{0X00700247, "US", "ShadowColorCIELabValue"},
{0X00700248, "CS", "Underlined"},
{0X00700249, "CS", "Bold"},
{0X00700250, "CS", "Italic"},
{0X00700251, "US", "PatternOnColorCIELabValue"},
{0X00700252, "US", "PatternOffColorCIELabValue"},
{0X00700253, "FL", "LineThickness"},
{0X00700254, "CS", "LineDashingStyle"},
{0X00700255, "UL", "LinePattern"},
{0X00700256, "OB", "FillPattern"},
{0X00700257, "CS", "FillMode"},
{0X00700258, "FL", "ShadowOpacity"},
{0X00700261, "FL", "GapLength"},
{0X00700262, "FL", "DiameterOfVisibility"},
{0X00700273, "FL", "RotationPoint"},
{0X00700274, "CS", "TickAlignment"},
{0X00700278, "CS", "ShowTickLabel"},
{0X00700279, "CS", "TickLabelAlignment"},
{0X00700282, "CS", "CompoundGraphicUnits"},
{0X00700284, "FL", "PatternOnOpacity"},
{0X00700285, "FL", "PatternOffOpacity"},
{0X00700287, "SQ", "MajorTicksSequence"},
{0X00700288, "FL", "TickPosition"},
{0X00700289, "SH", "TickLabel"},
{0X00700294, "CS", "CompoundGraphicType"},
{0X00700295, "UL", "GraphicGroupID"},
{0X00700306, "CS", "ShapeType"},
{0X00700308, "SQ", "RegistrationSequence"},
{0X00700309, "SQ", "MatrixRegistrationSequence"},
{0X0070030A, "SQ", "MatrixSequence"},
{0X0070030B, "FD", "FrameOfReferenceToDisplayedCoordinateSystemTransformationMatrix"},
{0X0070030C, "CS", "FrameOfReferenceTransformationMatrixType"},
{0X0070030D, "SQ", "RegistrationTypeCodeSequence"},
{0X0070030F, "ST", "FiducialDescription"},
{0X00700310, "SH", "FiducialIdentifier"},
{0X00700311, "SQ", "FiducialIdentifierCodeSequence"},
{0X00700312, "FD", "ContourUncertaintyRadius"},
{0X00700314, "SQ", "UsedFiducialsSequence"},
{0X00700318, "SQ", "GraphicCoordinatesDataSequence"},
{0X0070031A, "UI", "FiducialUID"},
{0X0070031B, "UI", "ReferencedFiducialUID"},
{0X0070031C, "SQ", "FiducialSetSequence"},
{0X0070031E, "SQ", "FiducialSequence"},
{0X0070031F, "SQ", "FiducialsPropertyCategoryCodeSequence"},
{0X00700401, "US", "GraphicLayerRecommendedDisplayCIELabValue"},
{0X00700402, "SQ", "BlendingSequence"},
{0X00700403, "FL", "RelativeOpacity"},
{0X00700404, "SQ", "ReferencedSpatialRegistrationSequence"},
{0X00700405, "CS", "BlendingPosition"},
{0X00701101, "UI", "PresentationDisplayCollectionUID"},
{0X00701102, "UI", "PresentationSequenceCollectionUID"},
{0X00701103, "US", "PresentationSequencePositionIndex"},
{0X00701104, "SQ", "RenderedImageReferenceSequence"},
{0X00701201, "SQ", "VolumetricPresentationStateInputSequence"},
{0X00701202, "CS", "PresentationInputType"},
{0X00701203, "US", "InputSequencePositionIndex"},
{0X00701204, "CS", "Crop"},
{0X00701205, "US", "CroppingSpecificationIndex"},
{0X00701206, "CS", "CompositingMethod"},
{0X00701207, "US", "VolumetricPresentationInputNumber"},
{0X00701208, "CS", "ImageVolumeGeometry"},
{0X00701209, "UI", "VolumetricPresentationInputSetUID"},
{0X0070120A, "SQ", "VolumetricPresentationInputSetSequence"},
{0X0070120B, "CS", "GlobalCrop"},
{0X0070120C, "US", "GlobalCroppingSpecificationIndex"},
{0X0070120D, "CS", "RenderingMethod"},
{0X00701301, "SQ", "VolumeCroppingSequence"},
{0X00701302, "CS", "VolumeCroppingMethod"},
{0X00701303, "FD", "BoundingBoxCrop"},
{0X00701304, "SQ", "ObliqueCroppingPlaneSequence"},
{0X00701305, "FD", "Plane"},
{0X00701306, "FD", "PlaneNormal"},
{0X00701309, "US", "CroppingSpecificationNumber"},
{0X00701501, "CS", "MultiPlanarReconstructionStyle"},
{0X00701502, "CS", "MPRThicknessType"},
{0X00701503, "FD", "MPRSlabThickness"},
{0X00701505, "FD", "MPRTopLeftHandCorner"},
{0X00701507, "FD", "MPRViewWidthDirection"},
{0X00701508, "FD", "MPRViewWidth"},
{0X0070150C, "UL", "NumberOfVolumetricCurvePoints"},
{0X0070150D, "OD", "VolumetricCurvePoints"},
{0X00701511, "FD", "MPRViewHeightDirection"},
{0X00701512, "FD", "MPRViewHeight"},
{0X00701602, "CS", "RenderProjection"},
{0X00701603, "FD", "ViewpointPosition"},
{0X00701604, "FD", "ViewpointLookAtPoint"},
{0X00701605, "FD", "ViewpointUpDirection"},
{0X00701606, "FD", "RenderFieldOfView"},
{0X00701607, "FD", "SamplingStepSize"},
{0X00701701, "CS", "ShadingStyle"},
{0X00701702, "FD", "AmbientReflectionIntensity"},
{0X00701703, "FD", "LightDirection"},
{0X00701704, "FD", "DiffuseReflectionIntensity"},
{0X00701705, "FD", "SpecularReflectionIntensity"},
{0X00701706, "FD", "Shininess"},
{0X00701801, "SQ", "PresentationStateClassificationComponentSequence"},
{0X00701802, "CS", "ComponentType"},
{0X00701803, "SQ", "ComponentInputSequence"},
{0X00701804, "US", "VolumetricPresentationInputIndex"},
{0X00701805, "SQ", "PresentationStateCompositorComponentSequence"},
{0X00701806, "SQ", "WeightingTransferFunctionSequence"},
{0X00701807, "US", "WeightingLookupTableDescriptor"},
{0X00701808, "OB", "WeightingLookupTableData"},
{0X00701901, "SQ", "VolumetricAnnotationSequence"},
{0X00701903, "SQ", "ReferencedStructuredContextSequence"},
{0X00701904, "UI", "ReferencedContentItem"},
{0X00701905, "SQ", "VolumetricPresentationInputAnnotationSequence"},
{0X00701907, "CS", "AnnotationClipping"},
{0X00701A01, "CS", "PresentationAnimationStyle"},
{0X00701A03, "FD", "RecommendedAnimationRate"},
{0X00701A04, "SQ", "AnimationCurveSequence"},
{0X00701A05, "FD", "AnimationStepSize"},
{0X00701A06, "FD", "SwivelRange"},
{0X00701A07, "OD", "VolumetricCurveUpDirections"},
{0X00701A08, "SQ", "VolumeStreamSequence"},
{0X00701A09, "LO", "RGBATransferFunctionDescription"},
{0X00701B01, "SQ", "AdvancedBlendingSequence"},
{0X00701B02, "US", "BlendingInputNumber"},
{0X00701B03, "SQ", "BlendingDisplayInputSequence"},
{0X00701B04, "SQ", "BlendingDisplaySequence"},
{0X00701B06, "CS", "BlendingMode"},
{0X00701B07, "CS", "TimeSeriesBlending"},
{0X00701B08, "CS", "GeometryForDisplay"},
{0X00701B11, "SQ", "ThresholdSequence"},
{0X00701B12, "SQ", "ThresholdValueSequence"},
{0X00701B13, "CS", "ThresholdType"},
{0X00701B14, "FD", "ThresholdValue"},
{0X00720002, "SH", "HangingProtocolName"},
{0X00720004, "LO", "HangingProtocolDescription"},
{0X00720006, "CS", "HangingProtocolLevel"},
{0X00720008, "LO", "HangingProtocolCreator"},
{0X0072000A, "DT", "HangingProtocolCreationDateTime"},
{0X0072000C, "SQ", "HangingProtocolDefinitionSequence"},
{0X0072000E, "SQ", "HangingProtocolUserIdentificationCodeSequence"},
{0X00720010, "LO", "HangingProtocolUserGroupName"},
{0X00720012, "SQ", "SourceHangingProtocolSequence"},
{0X00720014, "US", "NumberOfPriorsReferenced"},
{0X00720020, "SQ", "ImageSetsSequence"},
{0X00720022, "SQ", "ImageSetSelectorSequence"},
{0X00720024, "CS", "ImageSetSelectorUsageFlag"},
{0X00720026, "AT", "SelectorAttribute"},
{0X00720028, "US", "SelectorValueNumber"},
{0X00720030, "SQ", "TimeBasedImageSetsSequence"},
{0X00720032, "US", "ImageSetNumber"},
{0X00720034, "CS", "ImageSetSelectorCategory"},
{0X00720038, "US", "RelativeTime"},
{0X0072003A, "CS", "RelativeTimeUnits"},
{0X0072003C, "SS", "AbstractPriorValue"},
{0X0072003E, "SQ", "AbstractPriorCodeSequence"},
{0X00720040, "LO", "ImageSetLabel"},
{0X00720050, "CS", "SelectorAttributeVR"},
{0X00720052, "AT", "SelectorSequencePointer"},
{0X00720054, "LO", "SelectorSequencePointerPrivateCreator"},
{0X00720056, "LO", "SelectorAttributePrivateCreator"},
{0X0072005E, "AE", "SelectorAEValue"},
{0X0072005F, "AS", "SelectorASValue"},
{0X00720060, "AT", "SelectorATValue"},
{0X00720061, "DA", "SelectorDAValue"},
{0X00720062, "CS", "SelectorCSValue"},
{0X00720063, "DT", "SelectorDTValue"},
{0X00720064, "IS", "SelectorISValue"},
{0X00720065, "OB", "SelectorOBValue"},
{0X00720066, "LO", "SelectorLOValue"},
{0X00720067, "OF", "SelectorOFValue"},
{0X00720068, "LT", "SelectorLTValue"},
{0X00720069, "OW", "SelectorOWValue"},
{0X0072006A, "PN", "SelectorPNValue"},
{0X0072006B, "TM", "SelectorTMValue"},
{0X0072006C, "SH", "SelectorSHValue"},
{0X0072006D, "UN", "SelectorUNValue"},
{0X0072006E, "ST", "SelectorSTValue"},
{0X0072006F, "UC", "SelectorUCValue"},
{0X00720070, "UT", "SelectorUTValue"},
{0X00720071, "UR", "SelectorURValue"},
{0X00720072, "DS", "SelectorDSValue"},
{0X00720073, "OD", "SelectorODValue"},
{0X00720074, "FD", "SelectorFDValue"},
{0X00720075, "OL", "SelectorOLValue"},
{0X00720076, "FL", "SelectorFLValue"},
{0X00720078, "UL", "SelectorULValue"},
{0X0072007A, "US", "SelectorUSValue"},
{0X0072007C, "SL", "SelectorSLValue"},
{0X0072007E, "SS", "SelectorSSValue"},
{0X0072007F, "UI", "SelectorUIValue"},
{0X00720080, "SQ", "SelectorCodeSequenceValue"},
{0X00720081, "OV", "SelectorOVValue"},
{0X00720082, "SV", "SelectorSVValue"},
{0X00720083, "UV", "SelectorUVValue"},
{0X00720100, "US", "NumberOfScreens"},
{0X00720102, "SQ", "NominalScreenDefinitionSequence"},
{0X00720104, "US", "NumberOfVerticalPixels"},
{0X00720106, "US", "NumberOfHorizontalPixels"},
{0X00720108, "FD", "DisplayEnvironmentSpatialPosition"},
{0X0072010A, "US", "ScreenMinimumGrayscaleBitDepth"},
{0X0072010C, "US", "ScreenMinimumColorBitDepth"},
{0X0072010E, "US", "ApplicationMaximumRepaintTime"},
{0X00720200, "SQ", "DisplaySetsSequence"},
{0X00720202, "US", "DisplaySetNumber"},
{0X00720203, "LO", "DisplaySetLabel"},
{0X00720204, "US", "DisplaySetPresentationGroup"},
{0X00720206, "LO", "DisplaySetPresentationGroupDescription"},
{0X00720208, "CS", "PartialDataDisplayHandling"},
{0X00720210, "SQ", "SynchronizedScrollingSequence"},
{0X00720212, "US", "DisplaySetScrollingGroup"},
{0X00720214, "SQ", "NavigationIndicatorSequence"},
{0X00720216, "US", "NavigationDisplaySet"},
{0X00720218, "US", "ReferenceDisplaySets"},
{0X00720300, "SQ", "ImageBoxesSequence"},
{0X00720302, "US", "ImageBoxNumber"},
{0X00720304, "CS", "ImageBoxLayoutType"},
{0X00720306, "US", "ImageBoxTileHorizontalDimension"},
{0X00720308, "US", "ImageBoxTileVerticalDimension"},
{0X00720310, "CS", "ImageBoxScrollDirection"},
{0X00720312, "CS", "ImageBoxSmallScrollType"},
{0X00720314, "US", "ImageBoxSmallScrollAmount"},
{0X00720316, "CS", "ImageBoxLargeScrollType"},
{0X00720318, "US", "ImageBoxLargeScrollAmount"},
{0X00720320, "US", "ImageBoxOverlapPriority"},
{0X00720330, "FD", "CineRelativeToRealTime"},
{0X00720400, "SQ", "FilterOperationsSequence"},
{0X00720402, "CS", "FilterByCategory"},
{0X00720404, "CS", "FilterByAttributePresence"},
{0X00720406, "CS", "FilterByOperator"},
{0X00720420, "US", "StructuredDisplayBackgroundCIELabValue"},
{0X00720421, "US", "EmptyImageBoxCIELabValue"},
{0X00720422, "SQ", "StructuredDisplayImageBoxSequence"},
{0X00720424, "SQ", "StructuredDisplayTextBoxSequence"},
{0X00720427, "SQ", "ReferencedFirstFrameSequence"},
{0X00720430, "SQ", "ImageBoxSynchronizationSequence"},
{0X00720432, "US", "SynchronizedImageBoxList"},
{0X00720434, "CS", "TypeOfSynchronization"},
{0X00720500, "CS", "BlendingOperationType"},
{0X00720510, "CS", "ReformattingOperationType"},
{0X00720512, "FD", "ReformattingThickness"},
{0X00720514, "FD", "ReformattingInterval"},
{0X00720516, "CS", "ReformattingOperationInitialViewDirection"},
{0X00720520, "CS", "ThreeDRenderingType"},
{0X00720600, "SQ", "SortingOperationsSequence"},
{0X00720602, "CS", "SortByCategory"},
{0X00720604, "CS", "SortingDirection"},
{0X00720700, "CS", "DisplaySetPatientOrientation"},
{0X00720702, "CS", "VOIType"},
{0X00720704, "CS", "PseudoColorType"},
{0X00720705, "SQ", "PseudoColorPaletteInstanceReferenceSequence"},
{0X00720706, "CS", "ShowGrayscaleInverted"},
{0X00720710, "CS", "ShowImageTrueSizeFlag"},
{0X00720712, "CS", "ShowGraphicAnnotationFlag"},
{0X00720714, "CS", "ShowPatientDemographicsFlag"},
{0X00720716, "CS", "ShowAcquisitionTechniquesFlag"},
{0X00720717, "CS", "DisplaySetHorizontalJustification"},
{0X00720718, "CS", "DisplaySetVerticalJustification"},
{0X00740120, "FD", "ContinuationStartMeterset"},
{0X00740121, "FD", "ContinuationEndMeterset"},
{0X00741000, "CS", "ProcedureStepState"},
{0X00741002, "SQ", "ProcedureStepProgressInformationSequence"},
{0X00741004, "DS", "ProcedureStepProgress"},
{0X00741006, "ST", "ProcedureStepProgressDescription"},
{0X00741007, "SQ", "ProcedureStepProgressParametersSequence"},
{0X00741008, "SQ", "ProcedureStepCommunicationsURISequence"},
{0X0074100A, "UR", "ContactURI"},
{0X0074100C, "LO", "ContactDisplayName"},
{0X0074100E, "SQ", "ProcedureStepDiscontinuationReasonCodeSequence"},
{0X00741020, "SQ", "BeamTaskSequence"},
{0X00741022, "CS", "BeamTaskType"},
{0X00741024, "IS", "BeamOrderIndexTrial"},
{0X00741025, "CS", "AutosequenceFlag"},
{0X00741026, "FD", "TableTopVerticalAdjustedPosition"},
{0X00741027, "FD", "TableTopLongitudinalAdjustedPosition"},
{0X00741028, "FD", "TableTopLateralAdjustedPosition"},
{0X0074102A, "FD", "PatientSupportAdjustedAngle"},
{0X0074102B, "FD", "TableTopEccentricAdjustedAngle"},
{0X0074102C, "FD", "TableTopPitchAdjustedAngle"},
{0X0074102D, "FD", "TableTopRollAdjustedAngle"},
{0X00741030, "SQ", "DeliveryVerificationImageSequence"},
{0X00741032, "CS", "VerificationImageTiming"},
{0X00741034, "CS", "DoubleExposureFlag"},
{0X00741036, "CS", "DoubleExposureOrdering"},
{0X00741038, "DS", "DoubleExposureMetersetTrial"},
{0X0074103A, "DS", "DoubleExposureFieldDeltaTrial"},
{0X00741040, "SQ", "RelatedReferenceRTImageSequence"},
{0X00741042, "SQ", "GeneralMachineVerificationSequence"},
{0X00741044, "SQ", "ConventionalMachineVerificationSequence"},
{0X00741046, "SQ", "IonMachineVerificationSequence"},
{0X00741048, "SQ", "FailedAttributesSequence"},
{0X0074104A, "SQ", "OverriddenAttributesSequence"},
{0X0074104C, "SQ", "ConventionalControlPointVerificationSequence"},
{0X0074104E, "SQ", "IonControlPointVerificationSequence"},
{0X00741050, "SQ", "AttributeOccurrenceSequence"},
{0X00741052, "AT", "AttributeOccurrencePointer"},
{0X00741054, "UL", "AttributeItemSelector"},
{0X00741056, "LO", "AttributeOccurrencePrivateCreator"},
{0X00741057, "IS", "SelectorSequencePointerItems"},
{0X00741200, "CS", "ScheduledProcedureStepPriority"},
{0X00741202, "LO", "WorklistLabel"},
{0X00741204, "LO", "ProcedureStepLabel"},
{0X00741210, "SQ", "ScheduledProcessingParametersSequence"},
{0X00741212, "SQ", "PerformedProcessingParametersSequence"},
{0X00741216, "SQ", "UnifiedProcedureStepPerformedProcedureSequence"},
{0X00741220, "SQ", "RelatedProcedureStepSequence"},
{0X00741222, "LO", "ProcedureStepRelationshipType"},
{0X00741224, "SQ", "ReplacedProcedureStepSequence"},
{0X00741230, "LO", "DeletionLock"},
{0X00741234, "AE", "ReceivingAE"},
{0X00741236, "AE", "RequestingAE"},
{0X00741238, "LT", "ReasonForCancellation"},
{0X00741242, "CS", "SCPStatus"},
{0X00741244, "CS", "SubscriptionListStatus"},
{0X00741246, "CS", "UnifiedProcedureStepListStatus"},
{0X00741324, "UL", "BeamOrderIndex"},
{0X00741338, "FD", "DoubleExposureMeterset"},
{0X0074133A, "FD", "DoubleExposureFieldDelta"},
{0X00741401, "SQ", "BrachyTaskSequence"},
{0X00741402, "DS", "ContinuationStartTotalReferenceAirKerma"},
{0X00741403, "DS", "ContinuationEndTotalReferenceAirKerma"},
{0X00741404, "IS", "ContinuationPulseNumber"},
{0X00741405, "SQ", "ChannelDeliveryOrderSequence"},
{0X00741406, "IS", "ReferencedChannelNumber"},
{0X00741407, "DS", "StartCumulativeTimeWeight"},
{0X00741408, "DS", "EndCumulativeTimeWeight"},
{0X00741409, "SQ", "OmittedChannelSequence"},
{0X0074140A, "CS", "ReasonForChannelOmission"},
{0X0074140B, "LO", "ReasonForChannelOmissionDescription"},
{0X0074140C, "IS", "ChannelDeliveryOrderIndex"},
{0X0074140D, "SQ", "ChannelDeliveryContinuationSequence"},
{0X0074140E, "SQ", "OmittedApplicationSetupSequence"},
{0X00760001, "LO", "ImplantAssemblyTemplateName"},
{0X00760003, "LO", "ImplantAssemblyTemplateIssuer"},
{0X00760006, "LO", "ImplantAssemblyTemplateVersion"},
{0X00760008, "SQ", "ReplacedImplantAssemblyTemplateSequence"},
{0X0076000A, "CS", "ImplantAssemblyTemplateType"},
{0X0076000C, "SQ", "OriginalImplantAssemblyTemplateSequence"},
{0X0076000E, "SQ", "DerivationImplantAssemblyTemplateSequence"},
{0X00760010, "SQ", "ImplantAssemblyTemplateTargetAnatomySequence"},
{0X00760020, "SQ", "ProcedureTypeCodeSequence"},
{0X00760030, "LO", "SurgicalTechnique"},
{0X00760032, "SQ", "ComponentTypesSequence"},
{0X00760034, "SQ", "ComponentTypeCodeSequence"},
{0X00760036, "CS", "ExclusiveComponentType"},
{0X00760038, "CS", "MandatoryComponentType"},
{0X00760040, "SQ", "ComponentSequence"},
{0X00760055, "US", "ComponentID"},
{0X00760060, "SQ", "ComponentAssemblySequence"},
{0X00760070, "US", "Component1ReferencedID"},
{0X00760080, "US", "Component1ReferencedMatingFeatureSetID"},
{0X00760090, "US", "Component1ReferencedMatingFeatureID"},
{0X007600A0, "US", "Component2ReferencedID"},
{0X007600B0, "US", "Component2ReferencedMatingFeatureSetID"},
{0X007600C0, "US", "Component2ReferencedMatingFeatureID"},
{0X00780001, "LO", "ImplantTemplateGroupName"},
{0X00780010, "ST", "ImplantTemplateGroupDescription"},
{0X00780020, "LO", "ImplantTemplateGroupIssuer"},
{0X00780024, "LO", "ImplantTemplateGroupVersion"},
{0X00780026, "SQ", "ReplacedImplantTemplateGroupSequence"},
{0X00780028, "SQ", "ImplantTemplateGroupTargetAnatomySequence"},
{0X0078002A, "SQ", "ImplantTemplateGroupMembersSequence"},
{0X0078002E, "US", "ImplantTemplateGroupMemberID"},
{0X00780050, "FD", "ThreeDImplantTemplateGroupMemberMatchingPoint"},
{0X00780060, "FD", "ThreeDImplantTemplateGroupMemberMatchingAxes"},
{0X00780070, "SQ", "ImplantTemplateGroupMemberMatching2DCoordinatesSequence"},
{0X00780090, "FD", "TwoDImplantTemplateGroupMemberMatchingPoint"},
{0X007800A0, "FD", "TwoDImplantTemplateGroupMemberMatchingAxes"},
{0X007800B0, "SQ", "ImplantTemplateGroupVariationDimensionSequence"},
{0X007800B2, "LO", "ImplantTemplateGroupVariationDimensionName"},
{0X007800B4, "SQ", "ImplantTemplateGroupVariationDimensionRankSequence"},
{0X007800B6, "US", "ReferencedImplantTemplateGroupMemberID"},
{0X007800B8, "US", "ImplantTemplateGroupVariationDimensionRank"},
{0X00800001, "SQ", "SurfaceScanAcquisitionTypeCodeSequence"},
{0X00800002, "SQ", "SurfaceScanModeCodeSequence"},
{0X00800003, "SQ", "RegistrationMethodCodeSequence"},
{0X00800004, "FD", "ShotDurationTime"},
{0X00800005, "FD", "ShotOffsetTime"},
{0X00800006, "US", "SurfacePointPresentationValueData"},
{0X00800007, "US", "SurfacePointColorCIELabValueData"},
{0X00800008, "SQ", "UVMappingSequence"},
{0X00800009, "SH", "TextureLabel"},
{0X00800010, "OF", "UValueData"},
{0X00800011, "OF", "VValueData"},
{0X00800012, "SQ", "ReferencedTextureSequence"},
{0X00800013, "SQ", "ReferencedSurfaceDataSequence"},
{0X00820001, "CS", "AssessmentSummary"},
{0X00820003, "UT", "AssessmentSummaryDescription"},
{0X00820004, "SQ", "AssessedSOPInstanceSequence"},
{0X00820005, "SQ", "ReferencedComparisonSOPInstanceSequence"},
{0X00820006, "UL", "NumberOfAssessmentObservations"},
{0X00820007, "SQ", "AssessmentObservationsSequence"},
{0X00820008, "CS", "ObservationSignificance"},
{0X0082000A, "UT", "ObservationDescription"},
{0X0082000C, "SQ", "StructuredConstraintObservationSequence"},
{0X00820010, "SQ", "AssessedAttributeValueSequence"},
{0X00820016, "LO", "AssessmentSetID"},
{0X00820017, "SQ", "AssessmentRequesterSequence"},
{0X00820018, "LO", "SelectorAttributeName"},
{0X00820019, "LO", "SelectorAttributeKeyword"},
{0X00820021, "SQ", "AssessmentTypeCodeSequence"},
{0X00820022, "SQ", "ObservationBasisCodeSequence"},
{0X00820023, "LO", "AssessmentLabel"},
{0X00820032, "CS", "ConstraintType"},
{0X00820033, "UT", "SpecificationSelectionGuidance"},
{0X00820034, "SQ", "ConstraintValueSequence"},
{0X00820035, "SQ", "RecommendedDefaultValueSequence"},
{0X00820036, "CS", "ConstraintViolationSignificance"},
{0X00820037, "UT", "ConstraintViolationCondition"},
{0X00820038, "CS", "ModifiableConstraintFlag"},
{0X00880130, "SH", "StorageMediaFileSetID"},
{0X00880140, "UI", "StorageMediaFileSetUID"},
{0X00880200, "SQ", "IconImageSequence"},
{0X00880904, "LO", "TopicTitle"},
{0X00880906, "ST", "TopicSubject"},
{0X00880910, "LO", "TopicAuthor"},
{0X00880912, "LO", "TopicKeywords"},
{0X01000410, "CS", "SOPInstanceStatus"},
{0X01000420, "DT", "SOPAuthorizationDateTime"},
{0X01000424, "LT", "SOPAuthorizationComment"},
{0X01000426, "LO", "AuthorizationEquipmentCertificationNumber"},
{0X04000005, "US", "MACIDNumber"},
{0X04000010, "UI", "MACCalculationTransferSyntaxUID"},
{0X04000015, "CS", "MACAlgorithm"},
{0X04000020, "AT", "DataElementsSigned"},
{0X04000100, "UI", "DigitalSignatureUID"},
{0X04000105, "DT", "DigitalSignatureDateTime"},
{0X04000110, "CS", "CertificateType"},
{0X04000115, "OB", "CertificateOfSigner"},
{0X04000120, "OB", "Signature"},
{0X04000305, "CS", "CertifiedTimestampType"},
{0X04000310, "OB", "CertifiedTimestamp"},
{0X04000315, "FL", ""},
{0X04000401, "SQ", "DigitalSignaturePurposeCodeSequence"},
{0X04000402, "SQ", "ReferencedDigitalSignatureSequence"},
{0X04000403, "SQ", "ReferencedSOPInstanceMACSequence"},
{0X04000404, "OB", "MAC"},
{0X04000500, "SQ", "EncryptedAttributesSequence"},
{0X04000510, "UI", "EncryptedContentTransferSyntaxUID"},
{0X04000520, "OB", "EncryptedContent"},
{0X04000550, "SQ", "ModifiedAttributesSequence"},
{0X04000551, "SQ", "NonconformingModifiedAttributesSequence"},
{0X04000552, "OB", "NonconformingDataElementValue"},
{0X04000561, "SQ", "OriginalAttributesSequence"},
{0X04000562, "DT", "AttributeModificationDateTime"},
{0X04000563, "LO", "ModifyingSystem"},
{0X04000564, "LO", "SourceOfPreviousValues"},
{0X04000565, "CS", "ReasonForTheAttributeModification"},
{0X04000600, "CS", "InstanceOriginStatus"},
{0X20000010, "IS", "NumberOfCopies"},
{0X2000001E, "SQ", "PrinterConfigurationSequence"},
{0X20000020, "CS", "PrintPriority"},
{0X20000030, "CS", "MediumType"},
{0X20000040, "CS", "FilmDestination"},
{0X20000050, "LO", "FilmSessionLabel"},
{0X20000060, "IS", "MemoryAllocation"},
{0X20000061, "IS", "MaximumMemoryAllocation"},
{0X20000062, "CS", "ColorImagePrintingFlag"},
{0X20000063, "CS", "CollationFlag"},
{0X20000065, "CS", "AnnotationFlag"},
{0X20000067, "CS", "ImageOverlayFlag"},
{0X20000069, "CS", "PresentationLUTFlag"},
{0X2000006A, "CS", "ImageBoxPresentationLUTFlag"},
{0X200000A0, "US", "MemoryBitDepth"},
{0X200000A1, "US", "PrintingBitDepth"},
{0X200000A2, "SQ", "MediaInstalledSequence"},
{0X200000A4, "SQ", "OtherMediaAvailableSequence"},
{0X200000A8, "SQ", "SupportedImageDisplayFormatsSequence"},
{0X20000500, "SQ", "ReferencedFilmBoxSequence"},
{0X20000510, "SQ", "ReferencedStoredPrintSequence"},
{0X20100010, "ST", "ImageDisplayFormat"},
{0X20100030, "CS", "AnnotationDisplayFormatID"},
{0X20100040, "CS", "FilmOrientation"},
{0X20100050, "CS", "FilmSizeID"},
{0X20100052, "CS", "PrinterResolutionID"},
{0X20100054, "CS", "DefaultPrinterResolutionID"},
{0X20100060, "CS", "MagnificationType"},
{0X20100080, "CS", "SmoothingType"},
{0X201000A6, "CS", "DefaultMagnificationType"},
{0X201000A7, "CS", "OtherMagnificationTypesAvailable"},
{0X201000A8, "CS", "DefaultSmoothingType"},
{0X201000A9, "CS", "OtherSmoothingTypesAvailable"},
{0X20100100, "CS", "BorderDensity"},
{0X20100110, "CS", "EmptyImageDensity"},
{0X20100120, "US", "MinDensity"},
{0X20100130, "US", "MaxDensity"},
{0X20100140, "CS", "Trim"},
{0X20100150, "ST", "ConfigurationInformation"},
{0X20100152, "LT", "ConfigurationInformationDescription"},
{0X20100154, "IS", "MaximumCollatedFilms"},
{0X2010015E, "US", "Illumination"},
{0X20100160, "US", "ReflectedAmbientLight"},
{0X20100376, "DS", "PrinterPixelSpacing"},
{0X20100500, "SQ", "ReferencedFilmSessionSequence"},
{0X20100510, "SQ", "ReferencedImageBoxSequence"},
{0X20100520, "SQ", "ReferencedBasicAnnotationBoxSequence"},
{0X20200010, "US", "ImageBoxPosition"},
{0X20200020, "CS", "Polarity"},
{0X20200030, "DS", "RequestedImageSize"},
{0X20200040, "CS", "RequestedDecimateCropBehavior"},
{0X20200050, "CS", "RequestedResolutionID"},
{0X202000A0, "CS", "RequestedImageSizeFlag"},
{0X202000A2, "CS", "DecimateCropResult"},
{0X20200110, "SQ", "BasicGrayscaleImageSequence"},
{0X20200111, "SQ", "BasicColorImageSequence"},
{0X20200130, "SQ", "ReferencedImageOverlayBoxSequence"},
{0X20200140, "SQ", "ReferencedVOILUTBoxSequence"},
{0X20300010, "US", "AnnotationPosition"},
{0X20300020, "LO", "TextString"},
{0X20400010, "SQ", "ReferencedOverlayPlaneSequence"},
{0X20400011, "US", "ReferencedOverlayPlaneGroups"},
{0X20400020, "SQ", "OverlayPixelDataSequence"},
{0X20400060, "CS", "OverlayMagnificationType"},
{0X20400070, "CS", "OverlaySmoothingType"},
{0X20400072, "CS", "OverlayOrImageMagnification"},
{0X20400074, "US", "MagnifyToNumberOfColumns"},
{0X20400080, "CS", "OverlayForegroundDensity"},
{0X20400082, "CS", "OverlayBackgroundDensity"},
{0X20400090, "CS", "OverlayMode"},
{0X20400100, "CS", "ThresholdDensity"},
{0X20400500, "SQ", "ReferencedImageBoxSequenceRetired"},
{0X20500010, "SQ", "PresentationLUTSequence"},
{0X20500020, "CS", "PresentationLUTShape"},
{0X20500500, "SQ", "ReferencedPresentationLUTSequence"},
{0X21000010, "SH", "PrintJobID"},
{0X21000020, "CS", "ExecutionStatus"},
{0X21000030, "CS", "ExecutionStatusInfo"},
{0X21000040, "DA", "CreationDate"},
{0X21000050, "TM", "CreationTime"},
{0X21000070, "AE", "Originator"},
{0X21000140, "AE", "DestinationAE"},
{0X21000160, "SH", "OwnerID"},
{0X21000170, "IS", "NumberOfFilms"},
{0X21000500, "SQ", "ReferencedPrintJobSequencePullStoredPrint"},
{0X21100010, "CS", "PrinterStatus"},
{0X21100020, "CS", "PrinterStatusInfo"},
{0X21100030, "LO", "PrinterName"},
{0X21100099, "SH", "PrintQueueID"},
{0X21200010, "CS", "QueueStatus"},
{0X21200050, "SQ", "PrintJobDescriptionSequence"},
{0X21200070, "SQ", "ReferencedPrintJobSequence"},
{0X21300010, "SQ", "PrintManagementCapabilitiesSequence"},
{0X21300015, "SQ", "PrinterCharacteristicsSequence"},
{0X21300030, "SQ", "FilmBoxContentSequence"},
{0X21300040, "SQ", "ImageBoxContentSequence"},
{0X21300050, "SQ", "AnnotationContentSequence"},
{0X21300060, "SQ", "ImageOverlayBoxContentSequence"},
{0X21300080, "SQ", "PresentationLUTContentSequence"},
{0X213000A0, "SQ", "ProposedStudySequence"},
{0X213000C0, "SQ", "OriginalImageSequence"},
{0X22000001, "CS", "LabelUsingInformationExtractedFromInstances"},
{0X22000002, "UT", "LabelText"},
{0X22000003, "CS", "LabelStyleSelection"},
{0X22000004, "LT", "MediaDisposition"},
{0X22000005, "LT", "BarcodeValue"},
{0X22000006, "CS", "BarcodeSymbology"},
{0X22000007, "CS", "AllowMediaSplitting"},
{0X22000008, "CS", "IncludeNonDICOMObjects"},
{0X22000009, "CS", "IncludeDisplayApplication"},
{0X2200000A, "CS", "PreserveCompositeInstancesAfterMediaCreation"},
{0X2200000B, "US", "TotalNumberOfPiecesOfMediaCreated"},
{0X2200000C, "LO", "RequestedMediaApplicationProfile"},
{0X2200000D, "SQ", "ReferencedStorageMediaSequence"},
{0X2200000E, "AT", "FailureAttributes"},
{0X2200000F, "CS", "AllowLossyCompression"},
{0X22000020, "CS", "RequestPriority"},
{0X30020002, "SH", "RTImageLabel"},
{0X30020003, "LO", "RTImageName"},
{0X30020004, "ST", "RTImageDescription"},
{0X3002000A, "CS", "ReportedValuesOrigin"},
{0X3002000C, "CS", "RTImagePlane"},
{0X3002000D, "DS", "XRayImageReceptorTranslation"},
{0X3002000E, "DS", "XRayImageReceptorAngle"},
{0X30020010, "DS", "RTImageOrientation"},
{0X30020011, "DS", "ImagePlanePixelSpacing"},
{0X30020012, "DS", "RTImagePosition"},
{0X30020020, "SH", "RadiationMachineName"},
{0X30020022, "DS", "RadiationMachineSAD"},
{0X30020024, "DS", "RadiationMachineSSD"},
{0X30020026, "DS", "RTImageSID"},
{0X30020028, "DS", "SourceToReferenceObjectDistance"},
{0X30020029, "IS", "FractionNumber"},
{0X30020030, "SQ", "ExposureSequence"},
{0X30020032, "DS", "MetersetExposure"},
{0X30020034, "DS", "DiaphragmPosition"},
{0X30020040, "SQ", "FluenceMapSequence"},
{0X30020041, "CS", "FluenceDataSource"},
{0X30020042, "DS", "FluenceDataScale"},
{0X30020050, "SQ", "PrimaryFluenceModeSequence"},
{0X30020051, "CS", "FluenceMode"},
{0X30020052, "SH", "FluenceModeID"},
{0X30040001, "CS", "DVHType"},
{0X30040002, "CS", "DoseUnits"},
{0X30040004, "CS", "DoseType"},
{0X30040005, "CS", "SpatialTransformOfDose"},
{0X30040006, "LO", "DoseComment"},
{0X30040008, "DS", "NormalizationPoint"},
{0X3004000A, "CS", "DoseSummationType"},
{0X3004000C, "DS", "GridFrameOffsetVector"},
{0X3004000E, "DS", "DoseGridScaling"},
{0X30040010, "SQ", "RTDoseROISequence"},
{0X30040012, "DS", "DoseValue"},
{0X30040014, "CS", "TissueHeterogeneityCorrection"},
{0X30040040, "DS", "DVHNormalizationPoint"},
{0X30040042, "DS", "DVHNormalizationDoseValue"},
{0X30040050, "SQ", "DVHSequence"},
{0X30040052, "DS", "DVHDoseScaling"},
{0X30040054, "CS", "DVHVolumeUnits"},
{0X30040056, "IS", "DVHNumberOfBins"},
{0X30040058, "DS", "DVHData"},
{0X30040060, "SQ", "DVHReferencedROISequence"},
{0X30040062, "CS", "DVHROIContributionType"},
{0X30040070, "DS", "DVHMinimumDose"},
{0X30040072, "DS", "DVHMaximumDose"},
{0X30040074, "DS", "DVHMeanDose"},
{0X30060002, "SH", "StructureSetLabel"},
{0X30060004, "LO", "StructureSetName"},
{0X30060006, "ST", "StructureSetDescription"},
{0X30060008, "DA", "StructureSetDate"},
{0X30060009, "TM", "StructureSetTime"},
{0X30060010, "SQ", "ReferencedFrameOfReferenceSequence"},
{0X30060012, "SQ", "RTReferencedStudySequence"},
{0X30060014, "SQ", "RTReferencedSeriesSequence"},
{0X30060016, "SQ", "ContourImageSequence"},
{0X30060018, "SQ", "PredecessorStructureSetSequence"},
{0X30060020, "SQ", "StructureSetROISequence"},
{0X30060022, "IS", "ROINumber"},
{0X30060024, "UI", "ReferencedFrameOfReferenceUID"},
{0X30060026, "LO", "ROIName"},
{0X30060028, "ST", "ROIDescription"},
{0X3006002A, "IS", "ROIDisplayColor"},
{0X3006002C, "DS", "ROIVolume"},
{0X30060030, "SQ", "RTRelatedROISequence"},
{0X30060033, "CS", "RTROIRelationship"},
{0X30060036, "CS", "ROIGenerationAlgorithm"},
{0X30060037, "SQ", "ROIDerivationAlgorithmIdentificationSequence"},
{0X30060038, "LO", "ROIGenerationDescription"},
{0X30060039, "SQ", "ROIContourSequence"},
{0X30060040, "SQ", "ContourSequence"},
{0X30060042, "CS", "ContourGeometricType"},
{0X30060044, "DS", "ContourSlabThickness"},
{0X30060045, "DS", "ContourOffsetVector"},
{0X30060046, "IS", "NumberOfContourPoints"},
{0X30060048, "IS", "ContourNumber"},
{0X30060049, "IS", "AttachedContours"},
{0X3006004A, "SQ", "SourcePixelPlanesCharacteristicsSequence"},
{0X30060050, "DS", "ContourData"},
{0X30060080, "SQ", "RTROIObservationsSequence"},
{0X30060082, "IS", "ObservationNumber"},
{0X30060084, "IS", "ReferencedROINumber"},
{0X30060085, "SH", "ROIObservationLabel"},
{0X30060086, "SQ", "RTROIIdentificationCodeSequence"},
{0X30060088, "ST", "ROIObservationDescription"},
{0X300600A0, "SQ", "RelatedRTROIObservationsSequence"},
{0X300600A4, "CS", "RTROIInterpretedType"},
{0X300600A6, "PN", "ROIInterpreter"},
{0X300600B0, "SQ", "ROIPhysicalPropertiesSequence"},
{0X300600B2, "CS", "ROIPhysicalProperty"},
{0X300600B4, "DS", "ROIPhysicalPropertyValue"},
{0X300600B6, "SQ", "ROIElementalCompositionSequence"},
{0X300600B7, "US", "ROIElementalCompositionAtomicNumber"},
{0X300600B8, "FL", "ROIElementalCompositionAtomicMassFraction"},
{0X300600B9, "SQ", "AdditionalRTROIIdentificationCodeSequence"},
{0X300600C0, "SQ", "FrameOfReferenceRelationshipSequence"},
{0X300600C2, "UI", "RelatedFrameOfReferenceUID"},
{0X300600C4, "CS", "FrameOfReferenceTransformationType"},
{0X300600C6, "DS", "FrameOfReferenceTransformationMatrix"},
{0X300600C8, "LO", "FrameOfReferenceTransformationComment"},
{0X300600C9, "SQ", "PatientLocationCoordinatesSequence"},
{0X300600CA, "SQ", "PatientLocationCoordinatesCodeSequence"},
{0X300600CB, "SQ", "PatientSupportPositionSequence"},
{0X30080010, "SQ", "MeasuredDoseReferenceSequence"},
{0X30080012, "ST", "MeasuredDoseDescription"},
{0X30080014, "CS", "MeasuredDoseType"},
{0X30080016, "DS", "MeasuredDoseValue"},
{0X30080020, "SQ", "TreatmentSessionBeamSequence"},
{0X30080021, "SQ", "TreatmentSessionIonBeamSequence"},
{0X30080022, "IS", "CurrentFractionNumber"},
{0X30080024, "DA", "TreatmentControlPointDate"},
{0X30080025, "TM", "TreatmentControlPointTime"},
{0X3008002A, "CS", "TreatmentTerminationStatus"},
{0X3008002B, "SH", "TreatmentTerminationCode"},
{0X3008002C, "CS", "TreatmentVerificationStatus"},
{0X30080030, "SQ", "ReferencedTreatmentRecordSequence"},
{0X30080032, "DS", "SpecifiedPrimaryMeterset"},
{0X30080033, "DS", "SpecifiedSecondaryMeterset"},
{0X30080036, "DS", "DeliveredPrimaryMeterset"},
{0X30080037, "DS", "DeliveredSecondaryMeterset"},
{0X3008003A, "DS", "SpecifiedTreatmentTime"},
{0X3008003B, "DS", "DeliveredTreatmentTime"},
{0X30080040, "SQ", "ControlPointDeliverySequence"},
{0X30080041, "SQ", "IonControlPointDeliverySequence"},
{0X30080042, "DS", "SpecifiedMeterset"},
{0X30080044, "DS", "DeliveredMeterset"},
{0X30080045, "FL", "MetersetRateSet"},
{0X30080046, "FL", "MetersetRateDelivered"},
{0X30080047, "FL", "ScanSpotMetersetsDelivered"},
{0X30080048, "DS", "DoseRateDelivered"},
{0X30080050, "SQ", "TreatmentSummaryCalculatedDoseReferenceSequence"},
{0X30080052, "DS", "CumulativeDoseToDoseReference"},
{0X30080054, "DA", "FirstTreatmentDate"},
{0X30080056, "DA", "MostRecentTreatmentDate"},
{0X3008005A, "IS", "NumberOfFractionsDelivered"},
{0X30080060, "SQ", "OverrideSequence"},
{0X30080061, "AT", "ParameterSequencePointer"},
{0X30080062, "AT", "OverrideParameterPointer"},
{0X30080063, "IS", "ParameterItemIndex"},
{0X30080064, "IS", "MeasuredDoseReferenceNumber"},
{0X30080065, "AT", "ParameterPointer"},
{0X30080066, "ST", "OverrideReason"},
{0X30080067, "US", "ParameterValueNumber"},
{0X30080068, "SQ", "CorrectedParameterSequence"},
{0X3008006A, "FL", "CorrectionValue"},
{0X30080070, "SQ", "CalculatedDoseReferenceSequence"},
{0X30080072, "IS", "CalculatedDoseReferenceNumber"},
{0X30080074, "ST", "CalculatedDoseReferenceDescription"},
{0X30080076, "DS", "CalculatedDoseReferenceDoseValue"},
{0X30080078, "DS", "StartMeterset"},
{0X3008007A, "DS", "EndMeterset"},
{0X30080080, "SQ", "ReferencedMeasuredDoseReferenceSequence"},
{0X30080082, "IS", "ReferencedMeasuredDoseReferenceNumber"},
{0X30080090, "SQ", "ReferencedCalculatedDoseReferenceSequence"},
{0X30080092, "IS", "ReferencedCalculatedDoseReferenceNumber"},
{0X300800A0, "SQ", "BeamLimitingDeviceLeafPairsSequence"},
{0X300800B0, "SQ", "RecordedWedgeSequence"},
{0X300800C0, "SQ", "RecordedCompensatorSequence"},
{0X300800D0, "SQ", "RecordedBlockSequence"},
{0X300800E0, "SQ", "TreatmentSummaryMeasuredDoseReferenceSequence"},
{0X300800F0, "SQ", "RecordedSnoutSequence"},
{0X300800F2, "SQ", "RecordedRangeShifterSequence"},
{0X300800F4, "SQ", "RecordedLateralSpreadingDeviceSequence"},
{0X300800F6, "SQ", "RecordedRangeModulatorSequence"},
{0X30080100, "SQ", "RecordedSourceSequence"},
{0X30080105, "LO", "SourceSerialNumber"},
{0X30080110, "SQ", "TreatmentSessionApplicationSetupSequence"},
{0X30080116, "CS", "ApplicationSetupCheck"},
{0X30080120, "SQ", "RecordedBrachyAccessoryDeviceSequence"},
{0X30080122, "IS", "ReferencedBrachyAccessoryDeviceNumber"},
{0X30080130, "SQ", "RecordedChannelSequence"},
{0X30080132, "DS", "SpecifiedChannelTotalTime"},
{0X30080134, "DS", "DeliveredChannelTotalTime"},
{0X30080136, "IS", "SpecifiedNumberOfPulses"},
{0X30080138, "IS", "DeliveredNumberOfPulses"},
{0X3008013A, "DS", "SpecifiedPulseRepetitionInterval"},
{0X3008013C, "DS", "DeliveredPulseRepetitionInterval"},
{0X30080140, "SQ", "RecordedSourceApplicatorSequence"},
{0X30080142, "IS", "ReferencedSourceApplicatorNumber"},
{0X30080150, "SQ", "RecordedChannelShieldSequence"},
{0X30080152, "IS", "ReferencedChannelShieldNumber"},
{0X30080160, "SQ", "BrachyControlPointDeliveredSequence"},
{0X30080162, "DA", "SafePositionExitDate"},
{0X30080164, "TM", "SafePositionExitTime"},
{0X30080166, "DA", "SafePositionReturnDate"},
{0X30080168, "TM", "SafePositionReturnTime"},
{0X30080171, "SQ", "PulseSpecificBrachyControlPointDeliveredSequence"},
{0X30080172, "US", "PulseNumber"},
{0X30080173, "SQ", "BrachyPulseControlPointDeliveredSequence"},
{0X30080200, "CS", "CurrentTreatmentStatus"},
{0X30080202, "ST", "TreatmentStatusComment"},
{0X30080220, "SQ", "FractionGroupSummarySequence"},
{0X30080223, "IS", "ReferencedFractionNumber"},
{0X30080224, "CS", "FractionGroupType"},
{0X30080230, "CS", "BeamStopperPosition"},
{0X30080240, "SQ", "FractionStatusSummarySequence"},
{0X30080250, "DA", "TreatmentDate"},
{0X30080251, "TM", "TreatmentTime"},
{0X300A0002, "SH", "RTPlanLabel"},
{0X300A0003, "LO", "RTPlanName"},
{0X300A0004, "ST", "RTPlanDescription"},
{0X300A0006, "DA", "RTPlanDate"},
{0X300A0007, "TM", "RTPlanTime"},
{0X300A0009, "LO", "TreatmentProtocols"},
{0X300A000A, "CS", "PlanIntent"},
{0X300A000B, "LO", "TreatmentSites"},
{0X300A000C, "CS", "RTPlanGeometry"},
{0X300A000E, "ST", "PrescriptionDescription"},
{0X300A0010, "SQ", "DoseReferenceSequence"},
{0X300A0012, "IS", "DoseReferenceNumber"},
{0X300A0013, "UI", "DoseReferenceUID"},
{0X300A0014, "CS", "DoseReferenceStructureType"},
{0X300A0015, "CS", "NominalBeamEnergyUnit"},
{0X300A0016, "LO", "DoseReferenceDescription"},
{0X300A0018, "DS", "DoseReferencePointCoordinates"},
{0X300A001A, "DS", "NominalPriorDose"},
{0X300A0020, "CS", "DoseReferenceType"},
{0X300A0021, "DS", "ConstraintWeight"},
{0X300A0022, "DS", "DeliveryWarningDose"},
{0X300A0023, "DS", "DeliveryMaximumDose"},
{0X300A0025, "DS", "TargetMinimumDose"},
{0X300A0026, "DS", "TargetPrescriptionDose"},
{0X300A0027, "DS", "TargetMaximumDose"},
{0X300A0028, "DS", "TargetUnderdoseVolumeFraction"},
{0X300A002A, "DS", "OrganAtRiskFullVolumeDose"},
{0X300A002B, "DS", "OrganAtRiskLimitDose"},
{0X300A002C, "DS", "OrganAtRiskMaximumDose"},
{0X300A002D, "DS", "OrganAtRiskOverdoseVolumeFraction"},
{0X300A0040, "SQ", "ToleranceTableSequence"},
{0X300A0042, "IS", "ToleranceTableNumber"},
{0X300A0043, "SH", "ToleranceTableLabel"},
{0X300A0044, "DS", "GantryAngleTolerance"},
{0X300A0046, "DS", "BeamLimitingDeviceAngleTolerance"},
{0X300A0048, "SQ", "BeamLimitingDeviceToleranceSequence"},
{0X300A004A, "DS", "BeamLimitingDevicePositionTolerance"},
{0X300A004B, "FL", "SnoutPositionTolerance"},
{0X300A004C, "DS", "PatientSupportAngleTolerance"},
{0X300A004E, "DS", "TableTopEccentricAngleTolerance"},
{0X300A004F, "FL", "TableTopPitchAngleTolerance"},
{0X300A0050, "FL", "TableTopRollAngleTolerance"},
{0X300A0051, "DS", "TableTopVerticalPositionTolerance"},
{0X300A0052, "DS", "TableTopLongitudinalPositionTolerance"},
{0X300A0053, "DS", "TableTopLateralPositionTolerance"},
{0X300A0055, "CS", "RTPlanRelationship"},
{0X300A0070, "SQ", "FractionGroupSequence"},
{0X300A0071, "IS", "FractionGroupNumber"},
{0X300A0072, "LO", "FractionGroupDescription"},
{0X300A0078, "IS", "NumberOfFractionsPlanned"},
{0X300A0079, "IS", "NumberOfFractionPatternDigitsPerDay"},
{0X300A007A, "IS", "RepeatFractionCycleLength"},
{0X300A007B, "LT", "FractionPattern"},
{0X300A0080, "IS", "NumberOfBeams"},
{0X300A0082, "DS", "BeamDoseSpecificationPoint"},
{0X300A0083, "UI", "ReferencedDoseReferenceUID"},
{0X300A0084, "DS", "BeamDose"},
{0X300A0086, "DS", "BeamMeterset"},
{0X300A0088, "FL", "BeamDosePointDepth"},
{0X300A0089, "FL", "BeamDosePointEquivalentDepth"},
{0X300A008A, "FL", "BeamDosePointSSD"},
{0X300A008B, "CS", "BeamDoseMeaning"},
{0X300A008C, "SQ", "BeamDoseVerificationControlPointSequence"},
{0X300A008D, "FL", "AverageBeamDosePointDepth"},
{0X300A008E, "FL", "AverageBeamDosePointEquivalentDepth"},
{0X300A008F, "FL", "AverageBeamDosePointSSD"},
{0X300A0090, "CS", "BeamDoseType"},
{0X300A0091, "DS", "AlternateBeamDose"},
{0X300A0092, "CS", "AlternateBeamDoseType"},
{0X300A0093, "CS", "DepthValueAveragingFlag"},
{0X300A0094, "DS", "BeamDosePointSourceToExternalContourDistance"},
{0X300A00A0, "IS", "NumberOfBrachyApplicationSetups"},
{0X300A00A2, "DS", "BrachyApplicationSetupDoseSpecificationPoint"},
{0X300A00A4, "DS", "BrachyApplicationSetupDose"},
{0X300A00B0, "SQ", "BeamSequence"},
{0X300A00B2, "SH", "TreatmentMachineName"},
{0X300A00B3, "CS", "PrimaryDosimeterUnit"},
{0X300A00B4, "DS", "SourceAxisDistance"},
{0X300A00B6, "SQ", "BeamLimitingDeviceSequence"},
{0X300A00B8, "CS", "RTBeamLimitingDeviceType"},
{0X300A00BA, "DS", "SourceToBeamLimitingDeviceDistance"},
{0X300A00BB, "FL", "IsocenterToBeamLimitingDeviceDistance"},
{0X300A00BC, "IS", "NumberOfLeafJawPairs"},
{0X300A00BE, "DS", "LeafPositionBoundaries"},
{0X300A00C0, "IS", "BeamNumber"},
{0X300A00C2, "LO", "BeamName"},
{0X300A00C3, "ST", "BeamDescription"},
{0X300A00C4, "CS", "BeamType"},
{0X300A00C5, "FD", "BeamDeliveryDurationLimit"},
{0X300A00C6, "CS", "RadiationType"},
{0X300A00C7, "CS", "HighDoseTechniqueType"},
{0X300A00C8, "IS", "ReferenceImageNumber"},
{0X300A00CA, "SQ", "PlannedVerificationImageSequence"},
{0X300A00CC, "LO", "ImagingDeviceSpecificAcquisitionParameters"},
{0X300A00CE, "CS", "TreatmentDeliveryType"},
{0X300A00D0, "IS", "NumberOfWedges"},
{0X300A00D1, "SQ", "WedgeSequence"},
{0X300A00D2, "IS", "WedgeNumber"},
{0X300A00D3, "CS", "WedgeType"},
{0X300A00D4, "SH", "WedgeID"},
{0X300A00D5, "IS", "WedgeAngle"},
{0X300A00D6, "DS", "WedgeFactor"},
{0X300A00D7, "FL", "TotalWedgeTrayWaterEquivalentThickness"},
{0X300A00D8, "DS", "WedgeOrientation"},
{0X300A00D9, "FL", "IsocenterToWedgeTrayDistance"},
{0X300A00DA, "DS", "SourceToWedgeTrayDistance"},
{0X300A00DB, "FL", "WedgeThinEdgePosition"},
{0X300A00DC, "SH", "BolusID"},
{0X300A00DD, "ST", "BolusDescription"},
{0X300A00DE, "DS", "EffectiveWedgeAngle"},
{0X300A00E0, "IS", "NumberOfCompensators"},
{0X300A00E1, "SH", "MaterialID"},
{0X300A00E2, "DS", "TotalCompensatorTrayFactor"},
{0X300A00E3, "SQ", "CompensatorSequence"},
{0X300A00E4, "IS", "CompensatorNumber"},
{0X300A00E5, "SH", "CompensatorID"},
{0X300A00E6, "DS", "SourceToCompensatorTrayDistance"},
{0X300A00E7, "IS", "CompensatorRows"},
{0X300A00E8, "IS", "CompensatorColumns"},
{0X300A00E9, "DS", "CompensatorPixelSpacing"},
{0X300A00EA, "DS", "CompensatorPosition"},
{0X300A00EB, "DS", "CompensatorTransmissionData"},
{0X300A00EC, "DS", "CompensatorThicknessData"},
{0X300A00ED, "IS", "NumberOfBoli"},
{0X300A00EE, "CS", "CompensatorType"},
{0X300A00EF, "SH", "CompensatorTrayID"},
{0X300A00F0, "IS", "NumberOfBlocks"},
{0X300A00F2, "DS", "TotalBlockTrayFactor"},
{0X300A00F3, "FL", "TotalBlockTrayWaterEquivalentThickness"},
{0X300A00F4, "SQ", "BlockSequence"},
{0X300A00F5, "SH", "BlockTrayID"},
{0X300A00F6, "DS", "SourceToBlockTrayDistance"},
{0X300A00F7, "FL", "IsocenterToBlockTrayDistance"},
{0X300A00F8, "CS", "BlockType"},
{0X300A00F9, "LO", "AccessoryCode"},
{0X300A00FA, "CS", "BlockDivergence"},
{0X300A00FB, "CS", "BlockMountingPosition"},
{0X300A00FC, "IS", "BlockNumber"},
{0X300A00FE, "LO", "BlockName"},
{0X300A0100, "DS", "BlockThickness"},
{0X300A0102, "DS", "BlockTransmission"},
{0X300A0104, "IS", "BlockNumberOfPoints"},
{0X300A0106, "DS", "BlockData"},
{0X300A0107, "SQ", "ApplicatorSequence"},
{0X300A0108, "SH", "ApplicatorID"},
{0X300A0109, "CS", "ApplicatorType"},
{0X300A010A, "LO", "ApplicatorDescription"},
{0X300A010C, "DS", "CumulativeDoseReferenceCoefficient"},
{0X300A010E, "DS", "FinalCumulativeMetersetWeight"},
{0X300A0110, "IS", "NumberOfControlPoints"},
{0X300A0111, "SQ", "ControlPointSequence"},
{0X300A0112, "IS", "ControlPointIndex"},
{0X300A0114, "DS", "NominalBeamEnergy"},
{0X300A0115, "DS", "DoseRateSet"},
{0X300A0116, "SQ", "WedgePositionSequence"},
{0X300A0118, "CS", "WedgePosition"},
{0X300A011A, "SQ", "BeamLimitingDevicePositionSequence"},
{0X300A011C, "DS", "LeafJawPositions"},
{0X300A011E, "DS", "GantryAngle"},
{0X300A011F, "CS", "GantryRotationDirection"},
{0X300A0120, "DS", "BeamLimitingDeviceAngle"},
{0X300A0121, "CS", "BeamLimitingDeviceRotationDirection"},
{0X300A0122, "DS", "PatientSupportAngle"},
{0X300A0123, "CS", "PatientSupportRotationDirection"},
{0X300A0124, "DS", "TableTopEccentricAxisDistance"},
{0X300A0125, "DS", "TableTopEccentricAngle"},
{0X300A0126, "CS", "TableTopEccentricRotationDirection"},
{0X300A0128, "DS", "TableTopVerticalPosition"},
{0X300A0129, "DS", "TableTopLongitudinalPosition"},
{0X300A012A, "DS", "TableTopLateralPosition"},
{0X300A012C, "DS", "IsocenterPosition"},
{0X300A012E, "DS", "SurfaceEntryPoint"},
{0X300A0130, "DS", "SourceToSurfaceDistance"},
{0X300A0131, "FL", "AverageBeamDosePointSourceToExternalContourDistance"},
{0X300A0132, "FL", "SourceToExternalContourDistance"},
{0X300A0133, "FL", "ExternalContourEntryPoint"},
{0X300A0134, "DS", "CumulativeMetersetWeight"},
{0X300A0140, "FL", "TableTopPitchAngle"},
{0X300A0142, "CS", "TableTopPitchRotationDirection"},
{0X300A0144, "FL", "TableTopRollAngle"},
{0X300A0146, "CS", "TableTopRollRotationDirection"},
{0X300A0148, "FL", "HeadFixationAngle"},
{0X300A014A, "FL", "GantryPitchAngle"},
{0X300A014C, "CS", "GantryPitchRotationDirection"},
{0X300A014E, "FL", "GantryPitchAngleTolerance"},
{0X300A0150, "CS", "FixationEye"},
{0X300A0151, "DS", "ChairHeadFramePosition"},
{0X300A0152, "DS", "HeadFixationAngleTolerance"},
{0X300A0153, "DS", "ChairHeadFramePositionTolerance"},
{0X300A0154, "DS", "FixationLightAzimuthalAngleTolerance"},
{0X300A0155, "DS", "FixationLightPolarAngleTolerance"},
{0X300A0180, "SQ", "PatientSetupSequence"},
{0X300A0182, "IS", "PatientSetupNumber"},
{0X300A0183, "LO", "PatientSetupLabel"},
{0X300A0184, "LO", "PatientAdditionalPosition"},
{0X300A0190, "SQ", "FixationDeviceSequence"},
{0X300A0192, "CS", "FixationDeviceType"},
{0X300A0194, "SH", "FixationDeviceLabel"},
{0X300A0196, "ST", "FixationDeviceDescription"},
{0X300A0198, "SH", "FixationDevicePosition"},
{0X300A0199, "FL", "FixationDevicePitchAngle"},
{0X300A019A, "FL", "FixationDeviceRollAngle"},
{0X300A01A0, "SQ", "ShieldingDeviceSequence"},
{0X300A01A2, "CS", "ShieldingDeviceType"},
{0X300A01A4, "SH", "ShieldingDeviceLabel"},
{0X300A01A6, "ST", "ShieldingDeviceDescription"},
{0X300A01A8, "SH", "ShieldingDevicePosition"},
{0X300A01B0, "CS", "SetupTechnique"},
{0X300A01B2, "ST", "SetupTechniqueDescription"},
{0X300A01B4, "SQ", "SetupDeviceSequence"},
{0X300A01B6, "CS", "SetupDeviceType"},
{0X300A01B8, "SH", "SetupDeviceLabel"},
{0X300A01BA, "ST", "SetupDeviceDescription"},
{0X300A01BC, "DS", "SetupDeviceParameter"},
{0X300A01D0, "ST", "SetupReferenceDescription"},
{0X300A01D2, "DS", "TableTopVerticalSetupDisplacement"},
{0X300A01D4, "DS", "TableTopLongitudinalSetupDisplacement"},
{0X300A01D6, "DS", "TableTopLateralSetupDisplacement"},
{0X300A0200, "CS", "BrachyTreatmentTechnique"},
{0X300A0202, "CS", "BrachyTreatmentType"},
{0X300A0206, "SQ", "TreatmentMachineSequence"},
{0X300A0210, "SQ", "SourceSequence"},
{0X300A0212, "IS", "SourceNumber"},
{0X300A0214, "CS", "SourceType"},
{0X300A0216, "LO", "SourceManufacturer"},
{0X300A0218, "DS", "ActiveSourceDiameter"},
{0X300A021A, "DS", "ActiveSourceLength"},
{0X300A021B, "SH", "SourceModelID"},
{0X300A021C, "LO", "SourceDescription"},
{0X300A0222, "DS", "SourceEncapsulationNominalThickness"},
{0X300A0224, "DS", "SourceEncapsulationNominalTransmission"},
{0X300A0226, "LO", "SourceIsotopeName"},
{0X300A0228, "DS", "SourceIsotopeHalfLife"},
{0X300A0229, "CS", "SourceStrengthUnits"},
{0X300A022A, "DS", "ReferenceAirKermaRate"},
{0X300A022B, "DS", "SourceStrength"},
{0X300A022C, "DA", "SourceStrengthReferenceDate"},
{0X300A022E, "TM", "SourceStrengthReferenceTime"},
{0X300A0230, "SQ", "ApplicationSetupSequence"},
{0X300A0232, "CS", "ApplicationSetupType"},
{0X300A0234, "IS", "ApplicationSetupNumber"},
{0X300A0236, "LO", "ApplicationSetupName"},
{0X300A0238, "LO", "ApplicationSetupManufacturer"},
{0X300A0240, "IS", "TemplateNumber"},
{0X300A0242, "SH", "TemplateType"},
{0X300A0244, "LO", "TemplateName"},
{0X300A0250, "DS", "TotalReferenceAirKerma"},
{0X300A0260, "SQ", "BrachyAccessoryDeviceSequence"},
{0X300A0262, "IS", "BrachyAccessoryDeviceNumber"},
{0X300A0263, "SH", "BrachyAccessoryDeviceID"},
{0X300A0264, "CS", "BrachyAccessoryDeviceType"},
{0X300A0266, "LO", "BrachyAccessoryDeviceName"},
{0X300A026A, "DS", "BrachyAccessoryDeviceNominalThickness"},
{0X300A026C, "DS", "BrachyAccessoryDeviceNominalTransmission"},
{0X300A0271, "DS", "ChannelEffectiveLength"},
{0X300A0272, "DS", "ChannelInnerLength"},
{0X300A0273, "SH", "AfterloaderChannelID"},
{0X300A0274, "DS", "SourceApplicatorTipLength"},
{0X300A0280, "SQ", "ChannelSequence"},
{0X300A0282, "IS", "ChannelNumber"},
{0X300A0284, "DS", "ChannelLength"},
{0X300A0286, "DS", "ChannelTotalTime"},
{0X300A0288, "CS", "SourceMovementType"},
{0X300A028A, "IS", "NumberOfPulses"},
{0X300A028C, "DS", "PulseRepetitionInterval"},
{0X300A0290, "IS", "SourceApplicatorNumber"},
{0X300A0291, "SH", "SourceApplicatorID"},
{0X300A0292, "CS", "SourceApplicatorType"},
{0X300A0294, "LO", "SourceApplicatorName"},
{0X300A0296, "DS", "SourceApplicatorLength"},
{0X300A0298, "LO", "SourceApplicatorManufacturer"},
{0X300A029C, "DS", "SourceApplicatorWallNominalThickness"},
{0X300A029E, "DS", "SourceApplicatorWallNominalTransmission"},
{0X300A02A0, "DS", "SourceApplicatorStepSize"},
{0X300A02A1, "IS", "ApplicatorShapeReferencedROINumber"},
{0X300A02A2, "IS", "TransferTubeNumber"},
{0X300A02A4, "DS", "TransferTubeLength"},
{0X300A02B0, "SQ", "ChannelShieldSequence"},
{0X300A02B2, "IS", "ChannelShieldNumber"},
{0X300A02B3, "SH", "ChannelShieldID"},
{0X300A02B4, "LO", "ChannelShieldName"},
{0X300A02B8, "DS", "ChannelShieldNominalThickness"},
{0X300A02BA, "DS", "ChannelShieldNominalTransmission"},
{0X300A02C8, "DS", "FinalCumulativeTimeWeight"},
{0X300A02D0, "SQ", "BrachyControlPointSequence"},
{0X300A02D2, "DS", "ControlPointRelativePosition"},
{0X300A02D4, "DS", "ControlPoint3DPosition"},
{0X300A02D6, "DS", "CumulativeTimeWeight"},
{0X300A02E0, "CS", "CompensatorDivergence"},
{0X300A02E1, "CS", "CompensatorMountingPosition"},
{0X300A02E2, "DS", "SourceToCompensatorDistance"},
{0X300A02E3, "FL", "TotalCompensatorTrayWaterEquivalentThickness"},
{0X300A02E4, "FL", "IsocenterToCompensatorTrayDistance"},
{0X300A02E5, "FL", "CompensatorColumnOffset"},
{0X300A02E6, "FL", "IsocenterToCompensatorDistances"},
{0X300A02E7, "FL", "CompensatorRelativeStoppingPowerRatio"},
{0X300A02E8, "FL", "CompensatorMillingToolDiameter"},
{0X300A02EA, "SQ", "IonRangeCompensatorSequence"},
{0X300A02EB, "LT", "CompensatorDescription"},
{0X300A0302, "IS", "RadiationMassNumber"},
{0X300A0304, "IS", "RadiationAtomicNumber"},
{0X300A0306, "SS", "RadiationChargeState"},
{0X300A0308, "CS", "ScanMode"},
{0X300A0309, "CS", "ModulatedScanModeType"},
{0X300A030A, "FL", "VirtualSourceAxisDistances"},
{0X300A030C, "SQ", "SnoutSequence"},
{0X300A030D, "FL", "SnoutPosition"},
{0X300A030F, "SH", "SnoutID"},
{0X300A0312, "IS", "NumberOfRangeShifters"},
{0X300A0314, "SQ", "RangeShifterSequence"},
{0X300A0316, "IS", "RangeShifterNumber"},
{0X300A0318, "SH", "RangeShifterID"},
{0X300A0320, "CS", "RangeShifterType"},
{0X300A0322, "LO", "RangeShifterDescription"},
{0X300A0330, "IS", "NumberOfLateralSpreadingDevices"},
{0X300A0332, "SQ", "LateralSpreadingDeviceSequence"},
{0X300A0334, "IS", "LateralSpreadingDeviceNumber"},
{0X300A0336, "SH", "LateralSpreadingDeviceID"},
{0X300A0338, "CS", "LateralSpreadingDeviceType"},
{0X300A033A, "LO", "LateralSpreadingDeviceDescription"},
{0X300A033C, "FL", "LateralSpreadingDeviceWaterEquivalentThickness"},
{0X300A0340, "IS", "NumberOfRangeModulators"},
{0X300A0342, "SQ", "RangeModulatorSequence"},
{0X300A0344, "IS", "RangeModulatorNumber"},
{0X300A0346, "SH", "RangeModulatorID"},
{0X300A0348, "CS", "RangeModulatorType"},
{0X300A034A, "LO", "RangeModulatorDescription"},
{0X300A034C, "SH", "BeamCurrentModulationID"},
{0X300A0350, "CS", "PatientSupportType"},
{0X300A0352, "SH", "PatientSupportID"},
{0X300A0354, "LO", "PatientSupportAccessoryCode"},
{0X300A0355, "LO", "TrayAccessoryCode"},
{0X300A0356, "FL", "FixationLightAzimuthalAngle"},
{0X300A0358, "FL", "FixationLightPolarAngle"},
{0X300A035A, "FL", "MetersetRate"},
{0X300A0360, "SQ", "RangeShifterSettingsSequence"},
{0X300A0362, "LO", "RangeShifterSetting"},
{0X300A0364, "FL", "IsocenterToRangeShifterDistance"},
{0X300A0366, "FL", "RangeShifterWaterEquivalentThickness"},
{0X300A0370, "SQ", "LateralSpreadingDeviceSettingsSequence"},
{0X300A0372, "LO", "LateralSpreadingDeviceSetting"},
{0X300A0374, "FL", "IsocenterToLateralSpreadingDeviceDistance"},
{0X300A0380, "SQ", "RangeModulatorSettingsSequence"},
{0X300A0382, "FL", "RangeModulatorGatingStartValue"},
{0X300A0384, "FL", "RangeModulatorGatingStopValue"},
{0X300A0386, "FL", "RangeModulatorGatingStartWaterEquivalentThickness"},
{0X300A0388, "FL", "RangeModulatorGatingStopWaterEquivalentThickness"},
{0X300A038A, "FL", "IsocenterToRangeModulatorDistance"},
{0X300A038F, "FL", "ScanSpotTimeOffset"},
{0X300A0390, "SH", "ScanSpotTuneID"},
{0X300A0391, "IS", "ScanSpotPrescribedIndices"},
{0X300A0392, "IS", "NumberOfScanSpotPositions"},
{0X300A0393, "CS", "ScanSpotReordered"},
{0X300A0394, "FL", "ScanSpotPositionMap"},
{0X300A0395, "CS", "ScanSpotReorderingAllowed"},
{0X300A0396, "FL", "ScanSpotMetersetWeights"},
{0X300A0398, "FL", "ScanningSpotSize"},
{0X300A0399, "FL", "ScanSpotSizesDelivered"},
{0X300A039A, "IS", "NumberOfPaintings"},
{0X300A03A0, "SQ", "IonToleranceTableSequence"},
{0X300A03A2, "SQ", "IonBeamSequence"},
{0X300A03A4, "SQ", "IonBeamLimitingDeviceSequence"},
{0X300A03A6, "SQ", "IonBlockSequence"},
{0X300A03A8, "SQ", "IonControlPointSequence"},
{0X300A03AA, "SQ", "IonWedgeSequence"},
{0X300A03AC, "SQ", "IonWedgePositionSequence"},
{0X300A0401, "SQ", "ReferencedSetupImageSequence"},
{0X300A0402, "ST", "SetupImageComment"},
{0X300A0410, "SQ", "MotionSynchronizationSequence"},
{0X300A0412, "FL", "ControlPointOrientation"},
{0X300A0420, "SQ", "GeneralAccessorySequence"},
{0X300A0421, "SH", "GeneralAccessoryID"},
{0X300A0422, "ST", "GeneralAccessoryDescription"},
{0X300A0423, "CS", "GeneralAccessoryType"},
{0X300A0424, "IS", "GeneralAccessoryNumber"},
{0X300A0425, "FL", "SourceToGeneralAccessoryDistance"},
{0X300A0426, "DS", "IsocenterToGeneralAccessoryDistance"},
{0X300A0431, "SQ", "ApplicatorGeometrySequence"},
{0X300A0432, "CS", "ApplicatorApertureShape"},
{0X300A0433, "FL", "ApplicatorOpening"},
{0X300A0434, "FL", "ApplicatorOpeningX"},
{0X300A0435, "FL", "ApplicatorOpeningY"},
{0X300A0436, "FL", "SourceToApplicatorMountingPositionDistance"},
{0X300A0440, "IS", "NumberOfBlockSlabItems"},
{0X300A0441, "SQ", "BlockSlabSequence"},
{0X300A0442, "DS", "BlockSlabThickness"},
{0X300A0443, "US", "BlockSlabNumber"},
{0X300A0450, "SQ", "DeviceMotionControlSequence"},
{0X300A0451, "CS", "DeviceMotionExecutionMode"},
{0X300A0452, "CS", "DeviceMotionObservationMode"},
{0X300A0453, "SQ", "DeviceMotionParameterCodeSequence"},
{0X300A0501, "FL", "DistalDepthFraction"},
{0X300A0502, "FL", "DistalDepth"},
{0X300A0503, "FL", "NominalRangeModulationFractions"},
{0X300A0504, "FL", "NominalRangeModulatedRegionDepths"},
{0X300A0505, "SQ", "DepthDoseParametersSequence"},
{0X300A0506, "SQ", "DeliveredDepthDoseParametersSequence"},
{0X300A0507, "FL", "DeliveredDistalDepthFraction"},
{0X300A0508, "FL", "DeliveredDistalDepth"},
{0X300A0509, "FL", "DeliveredNominalRangeModulationFractions"},
{0X300A0510, "FL", "DeliveredNominalRangeModulatedRegionDepths"},
{0X300A0511, "CS", "DeliveredReferenceDoseDefinition"},
{0X300A0512, "CS", "ReferenceDoseDefinition"},
{0X300A0600, "US", "RTControlPointIndex"},
{0X300A0601, "US", "RadiationGenerationModeIndex"},
{0X300A0602, "US", "ReferencedDefinedDeviceIndex"},
{0X300A0603, "US", "RadiationDoseIdentificationIndex"},
{0X300A0604, "US", "NumberOfRTControlPoints"},
{0X300A0605, "US", "ReferencedRadiationGenerationModeIndex"},
{0X300A0606, "US", "TreatmentPositionIndex"},
{0X300A0607, "US", "ReferencedDeviceIndex"},
{0X300A0608, "LO", "TreatmentPositionGroupLabel"},
{0X300A0609, "UI", "TreatmentPositionGroupUID"},
{0X300A060A, "SQ", "TreatmentPositionGroupSequence"},
{0X300A060B, "US", "ReferencedTreatmentPositionIndex"},
{0X300A060C, "US", "ReferencedRadiationDoseIdentificationIndex"},
{0X300A060D, "FD", "RTAccessoryHolderWaterEquivalentThickness"},
{0X300A060E, "US", "ReferencedRTAccessoryHolderDeviceIndex"},
{0X300A060F, "CS", "RTAccessoryHolderSlotExistenceFlag"},
{0X300A0610, "SQ", "RTAccessoryHolderSlotSequence"},
{0X300A0611, "LO", "RTAccessoryHolderSlotID"},
{0X300A0612, "FD", "RTAccessoryHolderSlotDistance"},
{0X300A0613, "FD", "RTAccessorySlotDistance"},
{0X300A0614, "SQ", "RTAccessoryHolderDefinitionSequence"},
{0X300A0615, "LO", "RTAccessoryDeviceSlotID"},
{0X300A0616, "SQ", "RTRadiationSequence"},
{0X300A0617, "SQ", "RadiationDoseSequence"},
{0X300A0618, "SQ", "RadiationDoseIdentificationSequence"},
{0X300A0619, "LO", "RadiationDoseIdentificationLabel"},
{0X300A061A, "CS", "ReferenceDoseType"},
{0X300A061B, "CS", "PrimaryDoseValueIndicator"},
{0X300A061C, "SQ", "DoseValuesSequence"},
{0X300A061D, "CS", "DoseValuePurpose"},
{0X300A061E, "FD", "ReferenceDosePointCoordinates"},
{0X300A061F, "SQ", "RadiationDoseValuesParametersSequence"},
{0X300A0620, "SQ", "MetersetToDoseMappingSequence"},
{0X300A0621, "SQ", "ExpectedInVivoMeasurementValuesSequence"},
{0X300A0622, "US", "ExpectedInVivoMeasurementValueIndex"},
{0X300A0623, "LO", "RadiationDoseInVivoMeasurementLabel"},
{0X300A0624, "FD", "RadiationDoseCentralAxisDisplacement"},
{0X300A0625, "FD", "RadiationDoseValue"},
{0X300A0626, "FD", "RadiationDoseSourceToSkinDistance"},
{0X300A0627, "FD", "RadiationDoseMeasurementPointCoordinates"},
{0X300A0628, "FD", "RadiationDoseSourceToExternalContourDistance"},
{0X300A0629, "SQ", "RTToleranceSetSequence"},
{0X300A062A, "LO", "RTToleranceSetLabel"},
{0X300A062B, "SQ", "AttributeToleranceValuesSequence"},
{0X300A062C, "FD", "ToleranceValue"},
{0X300A062D, "SQ", "PatientSupportPositionToleranceSequence"},
{0X300A062E, "FD", "TreatmentTimeLimit"},
{0X300A062F, "SQ", "CArmPhotonElectronControlPointSequence"},
{0X300A0630, "SQ", "ReferencedRTRadiationSequence"},
{0X300A0631, "SQ", "ReferencedRTInstanceSequence"},
{0X300A0632, "SQ", "ReferencedRTPatientSetupSequence"},
{0X300A0634, "FD", "SourceToPatientSurfaceDistance"},
{0X300A0635, "SQ", "TreatmentMachineSpecialModeCodeSequence"},
{0X300A0636, "US", "IntendedNumberOfFractions"},
{0X300A0637, "CS", "RTRadiationSetIntent"},
{0X300A0638, "CS", "RTRadiationPhysicalAndGeometricContentDetailFlag"},
{0X300A0639, "CS", "RTRecordFlag"},
{0X300A063A, "SQ", "TreatmentDeviceIdentificationSequence"},
{0X300A063B, "SQ", "ReferencedRTPhysicianIntentSequence"},
{0X300A063C, "FD", "CumulativeMeterset"},
{0X300A063D, "FD", "DeliveryRate"},
{0X300A063E, "SQ", "DeliveryRateUnitSequence"},
{0X300A063F, "SQ", "TreatmentPositionSequence"},
{0X300A0640, "FD", "RadiationSourceAxisDistance"},
{0X300A0641, "US", "NumberOfRTBeamLimitingDevices"},
{0X300A0642, "FD", "RTBeamLimitingDeviceProximalDistance"},
{0X300A0643, "FD", "RTBeamLimitingDeviceDistalDistance"},
{0X300A0644, "SQ", "ParallelRTBeamDelimiterDeviceOrientationLabelCodeSequence"},
{0X300A0645, "FD", "BeamModifierOrientationAngle"},
{0X300A0646, "SQ", "FixedRTBeamDelimiterDeviceSequence"},
{0X300A0647, "SQ", "ParallelRTBeamDelimiterDeviceSequence"},
{0X300A0648, "US", "NumberOfParallelRTBeamDelimiters"},
{0X300A0649, "FD", "ParallelRTBeamDelimiterBoundaries"},
{0X300A064A, "FD", "ParallelRTBeamDelimiterPositions"},
{0X300A064B, "FD", "RTBeamLimitingDeviceOffset"},
{0X300A064C, "SQ", "RTBeamDelimiterGeometrySequence"},
{0X300A064D, "SQ", "RTBeamLimitingDeviceDefinitionSequence"},
{0X300A064E, "CS", "ParallelRTBeamDelimiterOpeningMode"},
{0X300A064F, "CS", "ParallelRTBeamDelimiterLeafMountingSide"},
{0X300A0650, "UI", "PatientSetupUID"},
{0X300A0651, "SQ", "WedgeDefinitionSequence"},
{0X300A0652, "FD", "RadiationBeamWedgeAngle"},
{0X300A0653, "FD", "RadiationBeamWedgeThinEdgeDistance"},
{0X300A0654, "FD", "RadiationBeamEffectiveWedgeAngle"},
{0X300A0655, "US", "NumberOfWedgePositions"},
{0X300A0656, "SQ", "RTBeamLimitingDeviceOpeningSequence"},
{0X300A0657, "US", "NumberOfRTBeamLimitingDeviceOpenings"},
{0X300A0658, "SQ", "RadiationDosimeterUnitSequence"},
{0X300A0659, "SQ", "RTDeviceDistanceReferenceLocationCodeSequence"},
{0X300A065A, "SQ", "RadiationDeviceConfigurationAndCommissioningKeySequence"},
{0X300A065B, "SQ", "PatientSupportPositionParameterSequence"},
{0X300A065C, "CS", "PatientSupportPositionSpecificationMethod"},
{0X300A065D, "SQ", "PatientSupportPositionDeviceParameterSequence"},
{0X300A065E, "US", "DeviceOrderIndex"},
{0X300A065F, "US", "PatientSupportPositionParameterOrderIndex"},
{0X300A0660, "SQ", "PatientSupportPositionDeviceToleranceSequence"},
{0X300A0661, "US", "PatientSupportPositionToleranceOrderIndex"},
{0X300A0662, "SQ", "CompensatorDefinitionSequence"},
{0X300A0663, "CS", "CompensatorMapOrientation"},
{0X300A0664, "OF", "CompensatorProximalThicknessMap"},
{0X300A0665, "OF", "CompensatorDistalThicknessMap"},
{0X300A0666, "FD", "CompensatorBasePlaneOffset"},
{0X300A0667, "SQ", "CompensatorShapeFabricationCodeSequence"},
{0X300A0668, "SQ", "CompensatorShapeSequence"},
{0X300A0669, "FD", "RadiationBeamCompensatorMillingToolDiameter"},
{0X300A066A, "SQ", "BlockDefinitionSequence"},
{0X300A066B, "OF", "BlockEdgeData"},
{0X300A066C, "CS", "BlockOrientation"},
{0X300A066D, "FD", "RadiationBeamBlockThickness"},
{0X300A066E, "FD", "RadiationBeamBlockSlabThickness"},
{0X300A066F, "SQ", "BlockEdgeDataSequence"},
{0X300A0670, "US", "NumberOfRTAccessoryHolders"},
{0X300A0671, "SQ", "GeneralAccessoryDefinitionSequence"},
{0X300A0672, "US", "NumberOfGeneralAccessories"},
{0X300A0673, "SQ", "BolusDefinitionSequence"},
{0X300A0674, "US", "NumberOfBoluses"},
{0X300A0675, "UI", "EquipmentFrameOfReferenceUID"},
{0X300A0676, "ST", "EquipmentFrameOfReferenceDescription"},
{0X300A0677, "SQ", "EquipmentReferencePointCoordinatesSequence"},
{0X300A0678, "SQ", "EquipmentReferencePointCodeSequence"},
{0X300A0679, "FD", "RTBeamLimitingDeviceAngle"},
{0X300A067A, "FD", "SourceRollAngle"},
{0X300A067B, "SQ", "RadiationGenerationModeSequence"},
{0X300A067C, "SH", "RadiationGenerationModeLabel"},
{0X300A067D, "ST", "RadiationGenerationModeDescription"},
{0X300A067E, "SQ", "RadiationGenerationModeMachineCodeSequence"},
{0X300A067F, "SQ", "RadiationTypeCodeSequence"},
{0X300A0680, "DS", "NominalEnergy"},
{0X300A0681, "DS", "MinimumNominalEnergy"},
{0X300A0682, "DS", "MaximumNominalEnergy"},
{0X300A0683, "SQ", "RadiationFluenceModifierCodeSequence"},
{0X300A0684, "SQ", "EnergyUnitCodeSequence"},
{0X300A0685, "US", "NumberOfRadiationGenerationModes"},
{0X300A0686, "SQ", "PatientSupportDevicesSequence"},
{0X300A0687, "US", "NumberOfPatientSupportDevices"},
{0X300A0688, "FD", "RTBeamModifierDefinitionDistance"},
{0X300A0689, "SQ", "BeamAreaLimitSequence"},
{0X300A068A, "SQ", "ReferencedRTPrescriptionSequence"},
{0X300A0700, "UI", "TreatmentSessionUID"},
{0X300A0701, "CS", "RTRadiationUsage"},
{0X300A0702, "SQ", "ReferencedRTRadiationSetSequence"},
{0X300A0703, "SQ", "ReferencedRTRadiationRecordSequence"},
{0X300A0704, "US", "RTRadiationSetDeliveryNumber"},
{0X300A0705, "US", "ClinicalFractionNumber"},
{0X300A0706, "CS", "RTTreatmentFractionCompletionStatus"},
{0X300A0707, "CS", "RTRadiationSetUsage"},
{0X300A0708, "CS", "TreatmentDeliveryContinuationFlag"},
{0X300A0709, "CS", "TreatmentRecordContentOrigin"},
{0X300A0714, "CS", "RTTreatmentTerminationStatus"},
{0X300A0715, "SQ", "RTTreatmentTerminationReasonCodeSequence"},
{0X300A0716, "SQ", "MachineSpecificTreatmentTerminationCodeSequence"},
{0X300A0722, "SQ", "RTRadiationSalvageRecordControlPointSequence"},
{0X300A0723, "CS", "StartingMetersetValueKnownFlag"},
{0X300A0730, "ST", "TreatmentTerminationDescription"},
{0X300A0731, "SQ", "TreatmentToleranceViolationSequence"},
{0X300A0732, "CS", "TreatmentToleranceViolationCategory"},
{0X300A0733, "SQ", "TreatmentToleranceViolationAttributeSequence"},
{0X300A0734, "ST", "TreatmentToleranceViolationDescription"},
{0X300A0735, "ST", "TreatmentToleranceViolationIdentification"},
{0X300A0736, "DT", "TreatmentToleranceViolationDateTime"},
{0X300A073A, "DT", "RecordedRTControlPointDateTime"},
{0X300A073B, "US", "ReferencedRadiationRTControlPointIndex"},
{0X300A073E, "SQ", "AlternateValueSequence"},
{0X300A073F, "SQ", "ConfirmationSequence"},
{0X300A0740, "SQ", "InterlockSequence"},
{0X300A0741, "DT", "InterlockDateTime"},
{0X300A0742, "ST", "InterlockDescription"},
{0X300A0743, "SQ", "InterlockOriginatingDeviceSequence"},
{0X300A0744, "SQ", "InterlockCodeSequence"},
{0X300A0745, "SQ", "InterlockResolutionCodeSequence"},
{0X300A0746, "SQ", "InterlockResolutionUserSequence"},
{0X300A0760, "DT", "OverrideDateTime"},
{0X300A0761, "SQ", "TreatmentToleranceViolationTypeCodeSequence"},
{0X300A0762, "SQ", "TreatmentToleranceViolationCauseCodeSequence"},
{0X300A0772, "SQ", "MeasuredMetersetToDoseMappingSequence"},
{0X300A0773, "US", "ReferencedExpectedInVivoMeasurementValueIndex"},
{0X300A0774, "SQ", "DoseMeasurementDeviceCodeSequence"},
{0X300A0780, "SQ", "AdditionalParameterRecordingInstanceSequence"},
{0X300A0782, "US", ""},
{0X300A0783, "ST", "InterlockOriginDescription"},
{0X300A0784, "SQ", "RTPatientPositionScopeSequence"},
{0X300A0785, "UI", "ReferencedTreatmentPositionGroupUID"},
{0X300A0786, "US", "RadiationOrderIndex"},
{0X300A0787, "SQ", "OmittedRadiationSequence"},
{0X300A0788, "SQ", "ReasonforOmissionCodeSequence"},
{0X300A0789, "SQ", "RTDeliveryStartPatientPositionSequence"},
{0X300A078A, "SQ", "RTTreatmentPreparationPatientPositionSequence"},
{0X300A078B, "SQ", "ReferencedRTTreatmentPreparationSequence"},
{0X300A078C, "SQ", "ReferencedPatientSetupPhotoSequence"},
{0X300A078D, "SQ", "PatientTreatmentPreparationMethodCodeSequence"},
{0X300A078E, "LT", "PatientTreatmentPreparationProcedureParameterDescription"},
{0X300A078F, "SQ", "PatientTreatmentPreparationDeviceSequence"},
{0X300A0790, "SQ", "PatientTreatmentPreparationProcedureSequence"},
{0X300A0791, "SQ", "PatientTreatmentPreparationProcedureCodeSequence"},
{0X300A0792, "LT", "PatientTreatmentPreparationMethodDescription"},
{0X300A0793, "SQ", "PatientTreatmentPreparationProcedureParameterSequence"},
{0X300A0794, "LT", "PatientSetupPhotoDescription"},
{0X300A0795, "US", "PatientTreatmentPreparationProcedureIndex"},
{0X300A0796, "US", "ReferencedPatientSetupProcedureIndex"},
{0X300A0797, "SQ", "RTRadiationTaskSequence"},
{0X300A0798, "SQ", "RTPatientPositionDisplacementSequence"},
{0X300A0799, "SQ", "RTPatientPositionSequence"},
{0X300A079A, "LO", "DisplacementReferenceLabel"},
{0X300A079B, "FD", "DisplacementMatrix"},
{0X300A079C, "SQ", "PatientSupportDisplacementSequence"},
{0X300A079D, "SQ", "DisplacementReferenceLocationCodeSequence"},
{0X300A079E, "CS", "RTRadiationSetDeliveryUsage"},
{0X300C0002, "SQ", "ReferencedRTPlanSequence"},
{0X300C0004, "SQ", "ReferencedBeamSequence"},
{0X300C0006, "IS", "ReferencedBeamNumber"},
{0X300C0007, "IS", "ReferencedReferenceImageNumber"},
{0X300C0008, "DS", "StartCumulativeMetersetWeight"},
{0X300C0009, "DS", "EndCumulativeMetersetWeight"},
{0X300C000A, "SQ", "ReferencedBrachyApplicationSetupSequence"},
{0X300C000C, "IS", "ReferencedBrachyApplicationSetupNumber"},
{0X300C000E, "IS", "ReferencedSourceNumber"},
{0X300C0020, "SQ", "ReferencedFractionGroupSequence"},
{0X300C0022, "IS", "ReferencedFractionGroupNumber"},
{0X300C0040, "SQ", "ReferencedVerificationImageSequence"},
{0X300C0042, "SQ", "ReferencedReferenceImageSequence"},
{0X300C0050, "SQ", "ReferencedDoseReferenceSequence"},
{0X300C0051, "IS", "ReferencedDoseReferenceNumber"},
{0X300C0055, "SQ", "BrachyReferencedDoseReferenceSequence"},
{0X300C0060, "SQ", "ReferencedStructureSetSequence"},
{0X300C006A, "IS", "ReferencedPatientSetupNumber"},
{0X300C0080, "SQ", "ReferencedDoseSequence"},
{0X300C00A0, "IS", "ReferencedToleranceTableNumber"},
{0X300C00B0, "SQ", "ReferencedBolusSequence"},
{0X300C00C0, "IS", "ReferencedWedgeNumber"},
{0X300C00D0, "IS", "ReferencedCompensatorNumber"},
{0X300C00E0, "IS", "ReferencedBlockNumber"},
{0X300C00F0, "IS", "ReferencedControlPointIndex"},
{0X300C00F2, "SQ", "ReferencedControlPointSequence"},
{0X300C00F4, "IS", "ReferencedStartControlPointIndex"},
{0X300C00F6, "IS", "ReferencedStopControlPointIndex"},
{0X300C0100, "IS", "ReferencedRangeShifterNumber"},
{0X300C0102, "IS", "ReferencedLateralSpreadingDeviceNumber"},
{0X300C0104, "IS", "ReferencedRangeModulatorNumber"},
{0X300C0111, "SQ", "OmittedBeamTaskSequence"},
{0X300C0112, "CS", "ReasonForOmission"},
{0X300C0113, "LO", "ReasonForOmissionDescription"},
{0X300E0002, "CS", "ApprovalStatus"},
{0X300E0004, "DA", "ReviewDate"},
{0X300E0005, "TM", "ReviewTime"},
{0X300E0008, "PN", "ReviewerName"},
{0X30100001, "SQ", "RadiobiologicalDoseEffectSequence"},
{0X30100002, "CS", "RadiobiologicalDoseEffectFlag"},
{0X30100003, "SQ", "EffectiveDoseCalculationMethodCategoryCodeSequence"},
{0X30100004, "SQ", "EffectiveDoseCalculationMethodCodeSequence"},
{0X30100005, "LO", "EffectiveDoseCalculationMethodDescription"},
{0X30100006, "UI", "ConceptualVolumeUID"},
{0X30100007, "SQ", "OriginatingSOPInstanceReferenceSequence"},
{0X30100008, "SQ", "ConceptualVolumeConstituentSequence"},
{0X30100009, "SQ", "EquivalentConceptualVolumeInstanceReferenceSequence"},
{0X3010000A, "SQ", "EquivalentConceptualVolumesSequence"},
{0X3010000B, "UI", "ReferencedConceptualVolumeUID"},
{0X3010000C, "UT", "ConceptualVolumeCombinationExpression"},
{0X3010000D, "US", "ConceptualVolumeConstituentIndex"},
{0X3010000E, "CS", "ConceptualVolumeCombinationFlag"},
{0X3010000F, "ST", "ConceptualVolumeCombinationDescription"},
{0X30100010, "CS", "ConceptualVolumeSegmentationDefinedFlag"},
{0X30100011, "SQ", "ConceptualVolumeSegmentationReferenceSequence"},
{0X30100012, "SQ", "ConceptualVolumeConstituentSegmentationReferenceSequence"},
{0X30100013, "UI", "ConstituentConceptualVolumeUID"},
{0X30100014, "SQ", "DerivationConceptualVolumeSequence"},
{0X30100015, "UI", "SourceConceptualVolumeUID"},
{0X30100016, "SQ", "ConceptualVolumeDerivationAlgorithmSequence"},
{0X30100017, "ST", "ConceptualVolumeDescription"},
{0X30100018, "SQ", "SourceConceptualVolumeSequence"},
{0X30100019, "SQ", "AuthorIdentificationSequence"},
{0X3010001A, "LO", "ManufacturerModelVersion"},
{0X3010001B, "UC", "DeviceAlternateIdentifier"},
{0X3010001C, "CS", "DeviceAlternateIdentifierType"},
{0X3010001D, "LT", "DeviceAlternateIdentifierFormat"},
{0X3010001E, "LO", "SegmentationCreationTemplateLabel"},
{0X3010001F, "UI", "SegmentationTemplateUID"},
{0X30100020, "US", "ReferencedSegmentReferenceIndex"},
{0X30100021, "SQ", "SegmentReferenceSequence"},
{0X30100022, "US", "SegmentReferenceIndex"},
{0X30100023, "SQ", "DirectSegmentReferenceSequence"},
{0X30100024, "SQ", "CombinationSegmentReferenceSequence"},
{0X30100025, "SQ", "ConceptualVolumeSequence"},
{0X30100026, "SQ", "SegmentedRTAccessoryDeviceSequence"},
{0X30100027, "SQ", "SegmentCharacteristicsSequence"},
{0X30100028, "SQ", "RelatedSegmentCharacteristicsSequence"},
{0X30100029, "US", "SegmentCharacteristicsPrecedence"},
{0X3010002A, "SQ", "RTSegmentAnnotationSequence"},
{0X3010002B, "SQ", "SegmentAnnotationCategoryCodeSequence"},
{0X3010002C, "SQ", "SegmentAnnotationTypeCodeSequence"},
{0X3010002D, "LO", "DeviceLabel"},
{0X3010002E, "SQ", "DeviceTypeCodeSequence"},
{0X3010002F, "SQ", "SegmentAnnotationTypeModifierCodeSequence"},
{0X30100030, "SQ", "PatientEquipmentRelationshipCodeSequence"},
{0X30100031, "UI", "ReferencedFiducialsUID"},
{0X30100032, "SQ", "PatientTreatmentOrientationSequence"},
{0X30100033, "SH", "UserContentLabel"},
{0X30100034, "LO", "UserContentLongLabel"},
{0X30100035, "SH", "EntityLabel"},
{0X30100036, "LO", "EntityName"},
{0X30100037, "ST", "EntityDescription"},
{0X30100038, "LO", "EntityLongLabel"},
{0X30100039, "US", "DeviceIndex"},
{0X3010003A, "US", "RTTreatmentPhaseIndex"},
{0X3010003B, "UI", "RTTreatmentPhaseUID"},
{0X3010003C, "US", "RTPrescriptionIndex"},
{0X3010003D, "US", "RTSegmentAnnotationIndex"},
{0X3010003E, "US", "BasisRTTreatmentPhaseIndex"},
{0X3010003F, "US", "RelatedRTTreatmentPhaseIndex"},
{0X30100040, "US", "ReferencedRTTreatmentPhaseIndex"},
{0X30100041, "US", "ReferencedRTPrescriptionIndex"},
{0X30100042, "US", "ReferencedParentRTPrescriptionIndex"},
{0X30100043, "ST", "ManufacturerDeviceIdentifier"},
{0X30100044, "SQ", "InstanceLevelReferencedPerformedProcedureStepSequence"},
{0X30100045, "CS", "RTTreatmentPhaseIntentPresenceFlag"},
{0X30100046, "CS", "RadiotherapyTreatmentType"},
{0X30100047, "CS", "TeletherapyRadiationType"},
{0X30100048, "CS", "BrachytherapySourceType"},
{0X30100049, "SQ", "ReferencedRTTreatmentPhaseSequence"},
{0X3010004A, "SQ", "ReferencedDirectSegmentInstanceSequence"},
{0X3010004B, "SQ", "IntendedRTTreatmentPhaseSequence"},
{0X3010004C, "DA", "IntendedPhaseStartDate"},
{0X3010004D, "DA", "IntendedPhaseEndDate"},
{0X3010004E, "SQ", "RTTreatmentPhaseIntervalSequence"},
{0X3010004F, "CS", "TemporalRelationshipIntervalAnchor"},
{0X30100050, "FD", "MinimumNumberOfIntervalDays"},
{0X30100051, "FD", "MaximumNumberOfIntervalDays"},
{0X30100052, "UI", "PertinentSOPClassesInStudy"},
{0X30100053, "UI", "PertinentSOPClassesInSeries"},
{0X30100054, "LO", "RTPrescriptionLabel"},
{0X30100055, "SQ", "RTPhysicianIntentPredecessorSequence"},
{0X30100056, "LO", "RTTreatmentApproachLabel"},
{0X30100057, "SQ", "RTPhysicianIntentSequence"},
{0X30100058, "US", "RTPhysicianIntentIndex"},
{0X30100059, "CS", "RTTreatmentIntentType"},
{0X3010005A, "UT", "RTPhysicianIntentNarrative"},
{0X3010005B, "SQ", "RTProtocolCodeSequence"},
{0X3010005C, "ST", "ReasonForSuperseding"},
{0X3010005D, "SQ", "RTDiagnosisCodeSequence"},
{0X3010005E, "US", "ReferencedRTPhysicianIntentIndex"},
{0X3010005F, "SQ", "RTPhysicianIntentInputInstanceSequence"},
{0X30100060, "SQ", "RTAnatomicPrescriptionSequence"},
{0X30100061, "UT", "PriorTreatmentDoseDescription"},
{0X30100062, "SQ", "PriorTreatmentReferenceSequence"},
{0X30100063, "CS", "DosimetricObjectiveEvaluationScope"},
{0X30100064, "SQ", "TherapeuticRoleCategoryCodeSequence"},
{0X30100065, "SQ", "TherapeuticRoleTypeCodeSequence"},
{0X30100066, "US", "ConceptualVolumeOptimizationPrecedence"},
{0X30100067, "SQ", "ConceptualVolumeCategoryCodeSequence"},
{0X30100068, "CS", "ConceptualVolumeBlockingConstraint"},
{0X30100069, "SQ", "ConceptualVolumeTypeCodeSequence"},
{0X3010006A, "SQ", "ConceptualVolumeTypeModifierCodeSequence"},
{0X3010006B, "SQ", "RTPrescriptionSequence"},
{0X3010006C, "SQ", "DosimetricObjectiveSequence"},
{0X3010006D, "SQ", "DosimetricObjectiveTypeCodeSequence"},
{0X3010006E, "UI", "DosimetricObjectiveUID"},
{0X3010006F, "UI", "ReferencedDosimetricObjectiveUID"},
{0X30100070, "SQ", "DosimetricObjectiveParameterSequence"},
{0X30100071, "SQ", "ReferencedDosimetricObjectivesSequence"},
{0X30100073, "CS", "AbsoluteDosimetricObjectiveFlag"},
{0X30100074, "FD", "DosimetricObjectiveWeight"},
{0X30100075, "CS", "DosimetricObjectivePurpose"},
{0X30100076, "SQ", "PlanningInputInformationSequence"},
{0X30100077, "LO", "TreatmentSite"},
{0X30100078, "SQ", "TreatmentSiteCodeSequence"},
{0X30100079, "SQ", "FractionPatternSequence"},
{0X3010007A, "UT", "TreatmentTechniqueNotes"},
{0X3010007B, "UT", "PrescriptionNotes"},
{0X3010007C, "IS", "NumberOfIntervalFractions"},
{0X3010007D, "US", "NumberOfFractions"},
{0X3010007E, "US", "IntendedDeliveryDuration"},
{0X3010007F, "UT", "FractionationNotes"},
{0X30100080, "SQ", "RTTreatmentTechniqueCodeSequence"},
{0X30100081, "SQ", "PrescriptionNotesSequence"},
{0X30100082, "SQ", "FractionBasedRelationshipSequence"},
{0X30100083, "CS", "FractionBasedRelationshipIntervalAnchor"},
{0X30100084, "FD", "MinimumHoursBetweenFractions"},
{0X30100085, "TM", "IntendedFractionStartTime"},
{0X30100086, "LT", "IntendedStartDayOfWeek"},
{0X30100087, "SQ", "WeekdayFractionPatternSequence"},
{0X30100088, "SQ", "DeliveryTimeStructureCodeSequence"},
{0X30100089, "SQ", "TreatmentSiteModifierCodeSequence"},
{0X30100090, "CS", "RoboticBaseLocationIndicator"},
{0X30100091, "SQ", "RoboticPathNodeSetCodeSequence"},
{0X30100092, "UL", "RoboticNodeIdentifier"},
{0X30100093, "FD", "RTTreatmentSourceCoordinates"},
{0X30100094, "FD", "RadiationSourceCoordinateSystemYawAngle"},
{0X30100095, "FD", "RadiationSourceCoordinateSystemRollAngle"},
{0X30100096, "FD", "RadiationSourceCoordinateSystemPitchAngle"},
{0X30100097, "SQ", "RoboticPathControlPointSequence"},
{0X30100098, "SQ", "TomotherapeuticControlPointSequence"},
{0X30100099, "FD", "TomotherapeuticLeafOpenDurations"},
{0X3010009A, "FD", "TomotherapeuticLeafInitialClosedDurations"},
{0X40000010, "LT", "Arbitrary"},
{0X40004000, "LT", "TextComments"},
{0X40080040, "SH", "ResultsID"},
{0X40080042, "LO", "ResultsIDIssuer"},
{0X40080050, "SQ", "ReferencedInterpretationSequence"},
{0X400800FF, "CS", "ReportProductionStatusTrial"},
{0X40080100, "DA", "InterpretationRecordedDate"},
{0X40080101, "TM", "InterpretationRecordedTime"},
{0X40080102, "PN", "InterpretationRecorder"},
{0X40080103, "LO", "ReferenceToRecordedSound"},
{0X40080108, "DA", "InterpretationTranscriptionDate"},
{0X40080109, "TM", "InterpretationTranscriptionTime"},
{0X4008010A, "PN", "InterpretationTranscriber"},
{0X4008010B, "ST", "InterpretationText"},
{0X4008010C, "PN", "InterpretationAuthor"},
{0X40080111, "SQ", "InterpretationApproverSequence"},
{0X40080112, "DA", "InterpretationApprovalDate"},
{0X40080113, "TM", "InterpretationApprovalTime"},
{0X40080114, "PN", "PhysicianApprovingInterpretation"},
{0X40080115, "LT", "InterpretationDiagnosisDescription"},
{0X40080117, "SQ", "InterpretationDiagnosisCodeSequence"},
{0X40080118, "SQ", "ResultsDistributionListSequence"},
{0X40080119, "PN", "DistributionName"},
{0X4008011A, "LO", "DistributionAddress"},
{0X40080200, "SH", "InterpretationID"},
{0X40080202, "LO", "InterpretationIDIssuer"},
{0X40080210, "CS", "InterpretationTypeID"},
{0X40080212, "CS", "InterpretationStatusID"},
{0X40080300, "ST", "Impressions"},
{0X40084000, "ST", "ResultsComments"},
{0X40100001, "CS", "LowEnergyDetectors"},
{0X40100002, "CS", "HighEnergyDetectors"},
{0X40100004, "SQ", "DetectorGeometrySequence"},
{0X40101001, "SQ", "ThreatROIVoxelSequence"},
{0X40101004, "FL", "ThreatROIBase"},
{0X40101005, "FL", "ThreatROIExtents"},
{0X40101006, "OB", "ThreatROIBitmap"},
{0X40101007, "SH", "RouteSegmentID"},
{0X40101008, "CS", "GantryType"},
{0X40101009, "CS", "OOIOwnerType"},
{0X4010100A, "SQ", "RouteSegmentSequence"},
{0X40101010, "US", "PotentialThreatObjectID"},
{0X40101011, "SQ", "ThreatSequence"},
{0X40101012, "CS", "ThreatCategory"},
{0X40101013, "LT", "ThreatCategoryDescription"},
{0X40101014, "CS", "ATDAbilityAssessment"},
{0X40101015, "CS", "ATDAssessmentFlag"},
{0X40101016, "FL", "ATDAssessmentProbability"},
{0X40101017, "FL", "Mass"},
{0X40101018, "FL", "Density"},
{0X40101019, "FL", "ZEffective"},
{0X4010101A, "SH", "BoardingPassID"},
{0X4010101B, "FL", "CenterOfMass"},
{0X4010101C, "FL", "CenterOfPTO"},
{0X4010101D, "FL", "BoundingPolygon"},
{0X4010101E, "SH", "RouteSegmentStartLocationID"},
{0X4010101F, "SH", "RouteSegmentEndLocationID"},
{0X40101020, "CS", "RouteSegmentLocationIDType"},
{0X40101021, "CS", "AbortReason"},
{0X40101023, "FL", "VolumeOfPTO"},
{0X40101024, "CS", "AbortFlag"},
{0X40101025, "DT", "RouteSegmentStartTime"},
{0X40101026, "DT", "RouteSegmentEndTime"},
{0X40101027, "CS", "TDRType"},
{0X40101028, "CS", "InternationalRouteSegment"},
{0X40101029, "LO", "ThreatDetectionAlgorithmAndVersion"},
{0X4010102A, "SH", "AssignedLocation"},
{0X4010102B, "DT", "AlarmDecisionTime"},
{0X40101031, "CS", "AlarmDecision"},
{0X40101033, "US", "NumberOfTotalObjects"},
{0X40101034, "US", "NumberOfAlarmObjects"},
{0X40101037, "SQ", "PTORepresentationSequence"},
{0X40101038, "SQ", "ATDAssessmentSequence"},
{0X40101039, "CS", "TIPType"},
{0X4010103A, "CS", "DICOSVersion"},
{0X40101041, "DT", "OOIOwnerCreationTime"},
{0X40101042, "CS", "OOIType"},
{0X40101043, "FL", "OOISize"},
{0X40101044, "CS", "AcquisitionStatus"},
{0X40101045, "SQ", "BasisMaterialsCodeSequence"},
{0X40101046, "CS", "PhantomType"},
{0X40101047, "SQ", "OOIOwnerSequence"},
{0X40101048, "CS", "ScanType"},
{0X40101051, "LO", "ItineraryID"},
{0X40101052, "SH", "ItineraryIDType"},
{0X40101053, "LO", "ItineraryIDAssigningAuthority"},
{0X40101054, "SH", "RouteID"},
{0X40101055, "SH", "RouteIDAssigningAuthority"},
{0X40101056, "CS", "InboundArrivalType"},
{0X40101058, "SH", "CarrierID"},
{0X40101059, "CS", "CarrierIDAssigningAuthority"},
{0X40101060, "FL", "SourceOrientation"},
{0X40101061, "FL", "SourcePosition"},
{0X40101062, "FL", "BeltHeight"},
{0X40101064, "SQ", "AlgorithmRoutingCodeSequence"},
{0X40101067, "CS", "TransportClassification"},
{0X40101068, "LT", "OOITypeDescriptor"},
{0X40101069, "FL", "TotalProcessingTime"},
{0X4010106C, "OB", "DetectorCalibrationData"},
{0X4010106D, "CS", "AdditionalScreeningPerformed"},
{0X4010106E, "CS", "AdditionalInspectionSelectionCriteria"},
{0X4010106F, "SQ", "AdditionalInspectionMethodSequence"},
{0X40101070, "CS", "AITDeviceType"},
{0X40101071, "SQ", "QRMeasurementsSequence"},
{0X40101072, "SQ", "TargetMaterialSequence"},
{0X40101073, "FD", "SNRThreshold"},
{0X40101075, "DS", "ImageScaleRepresentation"},
{0X40101076, "SQ", "ReferencedPTOSequence"},
{0X40101077, "SQ", "ReferencedTDRInstanceSequence"},
{0X40101078, "ST", "PTOLocationDescription"},
{0X40101079, "SQ", "AnomalyLocatorIndicatorSequence"},
{0X4010107A, "FL", "AnomalyLocatorIndicator"},
{0X4010107B, "SQ", "PTORegionSequence"},
{0X4010107C, "CS", "InspectionSelectionCriteria"},
{0X4010107D, "SQ", "SecondaryInspectionMethodSequence"},
{0X4010107E, "DS", "PRCSToRCSOrientation"},
{0X4FFE0001, "SQ", "MACParametersSequence"},
{0X52009229, "SQ", "SharedFunctionalGroupsSequence"},
{0X52009230, "SQ", "PerFrameFunctionalGroupsSequence"},
{0X54000100, "SQ", "WaveformSequence"},
{0X54000110, "OW", "ChannelMinimumValue"},
{0X54000112, "OW", "ChannelMaximumValue"},
{0X54001004, "US", "WaveformBitsAllocated"},
{0X54001006, "CS", "WaveformSampleInterpretation"},
{0X5400100A, "OW", "WaveformPaddingValue"},
{0X54001010, "OW", "WaveformData"},
{0X56000010, "OF", "FirstOrderPhaseCorrectionAngle"},
{0X56000020, "OF", "SpectroscopyData"},
{0X7FE00001, "OV", "ExtendedOffsetTable"},
{0X7FE00002, "OV", "ExtendedOffsetTableLengths"},
{0X7FE00008, "OF", "FloatPixelData"},
{0X7FE00009, "OD", "DoubleFloatPixelData"},
{0X7FE00010, "OW", "PixelData"},
{0X7FE00020, "OW", "CoefficientsSDVN"},
{0X7FE00030, "OW", "CoefficientsSDHN"},
{0X7FE00040, "OW", "CoefficientsSDDN"},
{0XFFFAFFFA, "SQ", "DigitalSignaturesSequence"},
{0XFFFCFFFC, "OB", "DataSetTrailingPadding"},
};


static int hash (uint32_t tag){    uint16_t group_number = (uint16_t)(tag >> 16);    switch (group_number) {
        case 0X0000:
            switch (tag) {
                case 0X00000000: return 0;
                case 0X00000001: return 1;
                case 0X00000002: return 2;
                case 0X00000003: return 3;
                case 0X00000010: return 4;
                case 0X00000100: return 5;
                case 0X00000110: return 6;
                case 0X00000120: return 7;
                case 0X00000200: return 8;
                case 0X00000300: return 9;
                case 0X00000400: return 10;
                case 0X00000600: return 11;
                case 0X00000700: return 12;
                case 0X00000800: return 13;
                case 0X00000850: return 14;
                case 0X00000860: return 15;
                case 0X00000900: return 16;
                case 0X00000901: return 17;
                case 0X00000902: return 18;
                case 0X00000903: return 19;
                case 0X00001000: return 20;
                case 0X00001001: return 21;
                case 0X00001002: return 22;
                case 0X00001005: return 23;
                case 0X00001008: return 24;
                case 0X00001020: return 25;
                case 0X00001021: return 26;
                case 0X00001022: return 27;
                case 0X00001023: return 28;
                case 0X00001030: return 29;
                case 0X00001031: return 30;
                case 0X00004000: return 31;
                case 0X00004010: return 32;
                case 0X00005010: return 33;
                case 0X00005020: return 34;
                case 0X00005110: return 35;
                case 0X00005120: return 36;
                case 0X00005130: return 37;
                case 0X00005140: return 38;
                case 0X00005150: return 39;
                case 0X00005160: return 40;
                case 0X00005170: return 41;
                case 0X00005180: return 42;
                case 0X00005190: return 43;
                case 0X000051A0: return 44;
                case 0X000051B0: return 45;
                default:         return -1;
            }

        case 0X0002:
            switch (tag) {
                case 0X00020000: return 46;
                case 0X00020001: return 47;
                case 0X00020002: return 48;
                case 0X00020003: return 49;
                case 0X00020010: return 50;
                case 0X00020012: return 51;
                case 0X00020013: return 52;
                case 0X00020016: return 53;
                case 0X00020017: return 54;
                case 0X00020018: return 55;
                case 0X00020026: return 56;
                case 0X00020027: return 57;
                case 0X00020028: return 58;
                case 0X00020031: return 59;
                case 0X00020032: return 60;
                case 0X00020033: return 61;
                case 0X00020035: return 62;
                case 0X00020036: return 63;
                case 0X00020037: return 64;
                case 0X00020038: return 65;
                case 0X00020100: return 66;
                case 0X00020102: return 67;
                default:         return -1;
            }

        case 0X0004:
            switch (tag) {
                case 0X00041130: return 68;
                case 0X00041141: return 69;
                case 0X00041142: return 70;
                case 0X00041200: return 71;
                case 0X00041202: return 72;
                case 0X00041212: return 73;
                case 0X00041220: return 74;
                case 0X00041400: return 75;
                case 0X00041410: return 76;
                case 0X00041420: return 77;
                case 0X00041430: return 78;
                case 0X00041432: return 79;
                case 0X00041500: return 80;
                case 0X00041504: return 81;
                case 0X00041510: return 82;
                case 0X00041511: return 83;
                case 0X00041512: return 84;
                case 0X0004151A: return 85;
                case 0X00041600: return 86;
                default:         return -1;
            }

        case 0X0008:
            switch (tag) {
                case 0X00080001: return 87;
                case 0X00080005: return 88;
                case 0X00080006: return 89;
                case 0X00080008: return 90;
                case 0X00080010: return 91;
                case 0X00080012: return 92;
                case 0X00080013: return 93;
                case 0X00080014: return 94;
                case 0X00080015: return 95;
                case 0X00080016: return 96;
                case 0X00080018: return 97;
                case 0X0008001A: return 98;
                case 0X0008001B: return 99;
                case 0X00080020: return 100;
                case 0X00080021: return 101;
                case 0X00080022: return 102;
                case 0X00080023: return 103;
                case 0X00080024: return 104;
                case 0X00080025: return 105;
                case 0X0008002A: return 106;
                case 0X00080030: return 107;
                case 0X00080031: return 108;
                case 0X00080032: return 109;
                case 0X00080033: return 110;
                case 0X00080034: return 111;
                case 0X00080035: return 112;
                case 0X00080040: return 113;
                case 0X00080041: return 114;
                case 0X00080042: return 115;
                case 0X00080050: return 116;
                case 0X00080051: return 117;
                case 0X00080052: return 118;
                case 0X00080053: return 119;
                case 0X00080054: return 120;
                case 0X00080055: return 121;
                case 0X00080056: return 122;
                case 0X00080058: return 123;
                case 0X00080060: return 124;
                case 0X00080061: return 125;
                case 0X00080062: return 126;
                case 0X00080063: return 127;
                case 0X00080064: return 128;
                case 0X00080068: return 129;
                case 0X00080070: return 130;
                case 0X00080080: return 131;
                case 0X00080081: return 132;
                case 0X00080082: return 133;
                case 0X00080090: return 134;
                case 0X00080092: return 135;
                case 0X00080094: return 136;
                case 0X00080096: return 137;
                case 0X0008009C: return 138;
                case 0X0008009D: return 139;
                case 0X00080100: return 140;
                case 0X00080101: return 141;
                case 0X00080102: return 142;
                case 0X00080103: return 143;
                case 0X00080104: return 144;
                case 0X00080105: return 145;
                case 0X00080106: return 146;
                case 0X00080107: return 147;
                case 0X00080108: return 148;
                case 0X00080109: return 149;
                case 0X0008010A: return 150;
                case 0X0008010B: return 151;
                case 0X0008010C: return 152;
                case 0X0008010D: return 153;
                case 0X0008010E: return 154;
                case 0X0008010F: return 155;
                case 0X00080110: return 156;
                case 0X00080112: return 157;
                case 0X00080114: return 158;
                case 0X00080115: return 159;
                case 0X00080116: return 160;
                case 0X00080117: return 161;
                case 0X00080118: return 162;
                case 0X00080119: return 163;
                case 0X00080120: return 164;
                case 0X00080121: return 165;
                case 0X00080122: return 166;
                case 0X00080123: return 167;
                case 0X00080124: return 168;
                case 0X00080201: return 169;
                case 0X00080202: return 170;
                case 0X00080220: return 171;
                case 0X00080221: return 172;
                case 0X00080222: return 173;
                case 0X00080300: return 174;
                case 0X00080301: return 175;
                case 0X00080302: return 176;
                case 0X00080303: return 177;
                case 0X00080304: return 178;
                case 0X00080305: return 179;
                case 0X00080306: return 180;
                case 0X00080307: return 181;
                case 0X00080308: return 182;
                case 0X00080309: return 183;
                case 0X0008030A: return 184;
                case 0X0008030B: return 185;
                case 0X0008030C: return 186;
                case 0X0008030D: return 187;
                case 0X0008030E: return 188;
                case 0X0008030F: return 189;
                case 0X00080310: return 190;
                case 0X00081000: return 191;
                case 0X00081010: return 192;
                case 0X00081030: return 193;
                case 0X00081032: return 194;
                case 0X0008103E: return 195;
                case 0X0008103F: return 196;
                case 0X00081040: return 197;
                case 0X00081041: return 198;
                case 0X00081048: return 199;
                case 0X00081049: return 200;
                case 0X00081050: return 201;
                case 0X00081052: return 202;
                case 0X00081060: return 203;
                case 0X00081062: return 204;
                case 0X00081070: return 205;
                case 0X00081072: return 206;
                case 0X00081080: return 207;
                case 0X00081084: return 208;
                case 0X00081090: return 209;
                case 0X00081100: return 210;
                case 0X00081110: return 211;
                case 0X00081111: return 212;
                case 0X00081115: return 213;
                case 0X00081120: return 214;
                case 0X00081125: return 215;
                case 0X00081130: return 216;
                case 0X00081134: return 217;
                case 0X0008113A: return 218;
                case 0X00081140: return 219;
                case 0X00081145: return 220;
                case 0X0008114A: return 221;
                case 0X0008114B: return 222;
                case 0X00081150: return 223;
                case 0X00081155: return 224;
                case 0X00081156: return 225;
                case 0X0008115A: return 226;
                case 0X00081160: return 227;
                case 0X00081161: return 228;
                case 0X00081162: return 229;
                case 0X00081163: return 230;
                case 0X00081164: return 231;
                case 0X00081167: return 232;
                case 0X00081190: return 233;
                case 0X00081195: return 234;
                case 0X00081196: return 235;
                case 0X00081197: return 236;
                case 0X00081198: return 237;
                case 0X00081199: return 238;
                case 0X0008119A: return 239;
                case 0X00081200: return 240;
                case 0X00081250: return 241;
                case 0X00082110: return 242;
                case 0X00082111: return 243;
                case 0X00082112: return 244;
                case 0X00082120: return 245;
                case 0X00082122: return 246;
                case 0X00082124: return 247;
                case 0X00082127: return 248;
                case 0X00082128: return 249;
                case 0X00082129: return 250;
                case 0X0008212A: return 251;
                case 0X00082130: return 252;
                case 0X00082132: return 253;
                case 0X00082133: return 254;
                case 0X00082134: return 255;
                case 0X00082135: return 256;
                case 0X00082142: return 257;
                case 0X00082143: return 258;
                case 0X00082144: return 259;
                case 0X00082200: return 260;
                case 0X00082204: return 261;
                case 0X00082208: return 262;
                case 0X00082218: return 263;
                case 0X00082220: return 264;
                case 0X00082228: return 265;
                case 0X00082229: return 266;
                case 0X00082230: return 267;
                case 0X00082240: return 268;
                case 0X00082242: return 269;
                case 0X00082244: return 270;
                case 0X00082246: return 271;
                case 0X00082251: return 272;
                case 0X00082253: return 273;
                case 0X00082255: return 274;
                case 0X00082256: return 275;
                case 0X00082257: return 276;
                case 0X00082258: return 277;
                case 0X00082259: return 278;
                case 0X0008225A: return 279;
                case 0X0008225C: return 280;
                case 0X00083001: return 281;
                case 0X00083002: return 282;
                case 0X00083010: return 283;
                case 0X00083011: return 284;
                case 0X00083012: return 285;
                case 0X00084000: return 286;
                case 0X00089007: return 287;
                case 0X00089092: return 288;
                case 0X00089121: return 289;
                case 0X00089123: return 290;
                case 0X00089124: return 291;
                case 0X00089154: return 292;
                case 0X00089205: return 293;
                case 0X00089206: return 294;
                case 0X00089207: return 295;
                case 0X00089208: return 296;
                case 0X00089209: return 297;
                case 0X00089215: return 298;
                case 0X00089237: return 299;
                case 0X00089410: return 300;
                case 0X00089458: return 301;
                case 0X00089459: return 302;
                case 0X00089460: return 303;
                default:         return -1;
            }

        case 0X0010:
            switch (tag) {
                case 0X00100010: return 304;
                case 0X00100020: return 305;
                case 0X00100021: return 306;
                case 0X00100022: return 307;
                case 0X00100024: return 308;
                case 0X00100026: return 309;
                case 0X00100027: return 310;
                case 0X00100028: return 311;
                case 0X00100030: return 312;
                case 0X00100032: return 313;
                case 0X00100033: return 314;
                case 0X00100034: return 315;
                case 0X00100035: return 316;
                case 0X00100040: return 317;
                case 0X00100050: return 318;
                case 0X00100101: return 319;
                case 0X00100102: return 320;
                case 0X00100200: return 321;
                case 0X00100201: return 322;
                case 0X00100212: return 323;
                case 0X00100213: return 324;
                case 0X00100214: return 325;
                case 0X00100215: return 326;
                case 0X00100216: return 327;
                case 0X00100217: return 328;
                case 0X00100218: return 329;
                case 0X00100219: return 330;
                case 0X00100221: return 331;
                case 0X00100222: return 332;
                case 0X00100223: return 333;
                case 0X00100229: return 334;
                case 0X00101000: return 335;
                case 0X00101001: return 336;
                case 0X00101002: return 337;
                case 0X00101005: return 338;
                case 0X00101010: return 339;
                case 0X00101020: return 340;
                case 0X00101021: return 341;
                case 0X00101022: return 342;
                case 0X00101023: return 343;
                case 0X00101024: return 344;
                case 0X00101030: return 345;
                case 0X00101040: return 346;
                case 0X00101050: return 347;
                case 0X00101060: return 348;
                case 0X00101080: return 349;
                case 0X00101081: return 350;
                case 0X00101090: return 351;
                case 0X00101100: return 352;
                case 0X00102000: return 353;
                case 0X00102110: return 354;
                case 0X00102150: return 355;
                case 0X00102152: return 356;
                case 0X00102154: return 357;
                case 0X00102155: return 358;
                case 0X00102160: return 359;
                case 0X00102180: return 360;
                case 0X001021A0: return 361;
                case 0X001021B0: return 362;
                case 0X001021C0: return 363;
                case 0X001021D0: return 364;
                case 0X001021F0: return 365;
                case 0X00102201: return 366;
                case 0X00102202: return 367;
                case 0X00102203: return 368;
                case 0X00102210: return 369;
                case 0X00102292: return 370;
                case 0X00102293: return 371;
                case 0X00102294: return 372;
                case 0X00102295: return 373;
                case 0X00102296: return 374;
                case 0X00102297: return 375;
                case 0X00102298: return 376;
                case 0X00102299: return 377;
                case 0X00104000: return 378;
                case 0X00109431: return 379;
                default:         return -1;
            }

        case 0X0012:
            switch (tag) {
                case 0X00120010: return 380;
                case 0X00120020: return 381;
                case 0X00120021: return 382;
                case 0X00120030: return 383;
                case 0X00120031: return 384;
                case 0X00120040: return 385;
                case 0X00120042: return 386;
                case 0X00120050: return 387;
                case 0X00120051: return 388;
                case 0X00120052: return 389;
                case 0X00120053: return 390;
                case 0X00120060: return 391;
                case 0X00120062: return 392;
                case 0X00120063: return 393;
                case 0X00120064: return 394;
                case 0X00120071: return 395;
                case 0X00120072: return 396;
                case 0X00120081: return 397;
                case 0X00120082: return 398;
                case 0X00120083: return 399;
                case 0X00120084: return 400;
                case 0X00120085: return 401;
                case 0X00120086: return 402;
                case 0X00120087: return 403;
                default:         return -1;
            }

        case 0X0014:
            switch (tag) {
                case 0X00140023: return 404;
                case 0X00140024: return 405;
                case 0X00140025: return 406;
                case 0X00140028: return 407;
                case 0X00140030: return 408;
                case 0X00140032: return 409;
                case 0X00140034: return 410;
                case 0X00140042: return 411;
                case 0X00140044: return 412;
                case 0X00140045: return 413;
                case 0X00140046: return 414;
                case 0X00140050: return 415;
                case 0X00140052: return 416;
                case 0X00140054: return 417;
                case 0X00140056: return 418;
                case 0X00140100: return 419;
                case 0X00140101: return 420;
                case 0X00140102: return 421;
                case 0X00140103: return 422;
                case 0X00140104: return 423;
                case 0X00140105: return 424;
                case 0X00140106: return 425;
                case 0X00140107: return 426;
                case 0X00140108: return 427;
                case 0X00141010: return 428;
                case 0X00141020: return 429;
                case 0X00141040: return 430;
                case 0X00142002: return 431;
                case 0X00142004: return 432;
                case 0X00142006: return 433;
                case 0X00142008: return 434;
                case 0X00142012: return 435;
                case 0X00142014: return 436;
                case 0X00142016: return 437;
                case 0X00142018: return 438;
                case 0X0014201A: return 439;
                case 0X0014201C: return 440;
                case 0X0014201E: return 441;
                case 0X00142030: return 442;
                case 0X00142032: return 443;
                case 0X00142202: return 444;
                case 0X00142204: return 445;
                case 0X00142206: return 446;
                case 0X00142208: return 447;
                case 0X0014220A: return 448;
                case 0X0014220C: return 449;
                case 0X0014220E: return 450;
                case 0X00142210: return 451;
                case 0X00142220: return 452;
                case 0X00142222: return 453;
                case 0X00142224: return 454;
                case 0X00142226: return 455;
                case 0X00142228: return 456;
                case 0X0014222A: return 457;
                case 0X0014222C: return 458;
                case 0X00143011: return 459;
                case 0X00143012: return 460;
                case 0X00143020: return 461;
                case 0X00143022: return 462;
                case 0X00143024: return 463;
                case 0X00143026: return 464;
                case 0X00143028: return 465;
                case 0X00143040: return 466;
                case 0X00143050: return 467;
                case 0X00143060: return 468;
                case 0X00143070: return 469;
                case 0X00143071: return 470;
                case 0X00143072: return 471;
                case 0X00143073: return 472;
                case 0X00143074: return 473;
                case 0X00143075: return 474;
                case 0X00143076: return 475;
                case 0X00143077: return 476;
                case 0X00143080: return 477;
                case 0X00143099: return 478;
                case 0X00143100: return 479;
                case 0X00143101: return 480;
                case 0X00144002: return 481;
                case 0X00144004: return 482;
                case 0X00144006: return 483;
                case 0X00144008: return 484;
                case 0X0014400A: return 485;
                case 0X0014400C: return 486;
                case 0X0014400E: return 487;
                case 0X0014400F: return 488;
                case 0X00144010: return 489;
                case 0X00144011: return 490;
                case 0X00144012: return 491;
                case 0X00144013: return 492;
                case 0X00144014: return 493;
                case 0X00144015: return 494;
                case 0X00144016: return 495;
                case 0X00144017: return 496;
                case 0X00144018: return 497;
                case 0X00144019: return 498;
                case 0X0014401A: return 499;
                case 0X0014401B: return 500;
                case 0X0014401C: return 501;
                case 0X0014401D: return 502;
                case 0X00144020: return 503;
                case 0X00144022: return 504;
                case 0X00144024: return 505;
                case 0X00144026: return 506;
                case 0X00144028: return 507;
                case 0X00144030: return 508;
                case 0X00144031: return 509;
                case 0X00144032: return 510;
                case 0X00144033: return 511;
                case 0X00144034: return 512;
                case 0X00144035: return 513;
                case 0X00144036: return 514;
                case 0X00144038: return 515;
                case 0X0014403A: return 516;
                case 0X0014403C: return 517;
                case 0X00144040: return 518;
                case 0X00144050: return 519;
                case 0X00144051: return 520;
                case 0X00144052: return 521;
                case 0X00144054: return 522;
                case 0X00144056: return 523;
                case 0X00144057: return 524;
                case 0X00144058: return 525;
                case 0X00144059: return 526;
                case 0X0014405A: return 527;
                case 0X0014405C: return 528;
                case 0X00144060: return 529;
                case 0X00144062: return 530;
                case 0X00144064: return 531;
                case 0X00144070: return 532;
                case 0X00144072: return 533;
                case 0X00144074: return 534;
                case 0X00144076: return 535;
                case 0X00144078: return 536;
                case 0X0014407A: return 537;
                case 0X0014407C: return 538;
                case 0X0014407E: return 539;
                case 0X00144080: return 540;
                case 0X00144081: return 541;
                case 0X00144082: return 542;
                case 0X00144083: return 543;
                case 0X00144084: return 544;
                case 0X00144085: return 545;
                case 0X00144086: return 546;
                case 0X00144087: return 547;
                case 0X00144088: return 548;
                case 0X00144089: return 549;
                case 0X0014408B: return 550;
                case 0X0014408C: return 551;
                case 0X0014408D: return 552;
                case 0X0014408E: return 553;
                case 0X00144091: return 554;
                case 0X00144092: return 555;
                case 0X0014409A: return 556;
                case 0X0014409B: return 557;
                case 0X0014409C: return 558;
                case 0X0014409D: return 559;
                case 0X0014409F: return 560;
                case 0X001440A0: return 561;
                case 0X001440A1: return 562;
                case 0X001440A2: return 563;
                case 0X00145002: return 564;
                case 0X00145004: return 565;
                case 0X00145100: return 566;
                case 0X00145101: return 567;
                case 0X00145102: return 568;
                case 0X00145103: return 569;
                case 0X00145104: return 570;
                case 0X00145105: return 571;
                case 0X00145106: return 572;
                case 0X00145107: return 573;
                case 0X00145108: return 574;
                case 0X00145109: return 575;
                case 0X0014510A: return 576;
                case 0X0014510B: return 577;
                case 0X0014510C: return 578;
                case 0X0014510D: return 579;
                case 0X0014510E: return 580;
                case 0X0014510F: return 581;
                case 0X00145110: return 582;
                case 0X00145111: return 583;
                case 0X00145112: return 584;
                case 0X00145113: return 585;
                case 0X00145114: return 586;
                case 0X00145115: return 587;
                case 0X00145116: return 588;
                case 0X00145117: return 589;
                case 0X00145118: return 590;
                case 0X00145119: return 591;
                case 0X0014511A: return 592;
                case 0X0014511B: return 593;
                case 0X0014511C: return 594;
                case 0X0014511D: return 595;
                case 0X0014511E: return 596;
                case 0X0014511F: return 597;
                default:         return -1;
            }

        case 0X0016:
            switch (tag) {
                case 0X00160001: return 598;
                case 0X00160002: return 599;
                case 0X00160003: return 600;
                case 0X00160004: return 601;
                case 0X00160005: return 602;
                case 0X00160006: return 603;
                case 0X00160007: return 604;
                case 0X00160008: return 605;
                case 0X00160009: return 606;
                case 0X0016000A: return 607;
                case 0X0016000B: return 608;
                case 0X0016000C: return 609;
                case 0X0016000D: return 610;
                case 0X0016000E: return 611;
                case 0X0016000F: return 612;
                case 0X00160010: return 613;
                case 0X00160011: return 614;
                case 0X00160012: return 615;
                case 0X00160013: return 616;
                case 0X00160014: return 617;
                case 0X00160015: return 618;
                case 0X00160016: return 619;
                case 0X00160017: return 620;
                case 0X00160018: return 621;
                case 0X00160019: return 622;
                case 0X0016001A: return 623;
                case 0X0016001B: return 624;
                case 0X0016001C: return 625;
                case 0X0016001D: return 626;
                case 0X0016001E: return 627;
                case 0X0016001F: return 628;
                case 0X00160020: return 629;
                case 0X00160021: return 630;
                case 0X00160022: return 631;
                case 0X00160023: return 632;
                case 0X00160024: return 633;
                case 0X00160025: return 634;
                case 0X00160026: return 635;
                case 0X00160027: return 636;
                case 0X00160028: return 637;
                case 0X00160029: return 638;
                case 0X0016002A: return 639;
                case 0X0016002B: return 640;
                case 0X00160030: return 641;
                case 0X00160031: return 642;
                case 0X00160032: return 643;
                case 0X00160033: return 644;
                case 0X00160034: return 645;
                case 0X00160035: return 646;
                case 0X00160036: return 647;
                case 0X00160037: return 648;
                case 0X00160038: return 649;
                case 0X00160039: return 650;
                case 0X0016003A: return 651;
                case 0X0016003B: return 652;
                case 0X00160041: return 653;
                case 0X00160042: return 654;
                case 0X00160043: return 655;
                case 0X00160044: return 656;
                case 0X00160045: return 657;
                case 0X00160046: return 658;
                case 0X00160047: return 659;
                case 0X00160048: return 660;
                case 0X00160049: return 661;
                case 0X0016004A: return 662;
                case 0X0016004B: return 663;
                case 0X0016004C: return 664;
                case 0X0016004D: return 665;
                case 0X0016004E: return 666;
                case 0X0016004F: return 667;
                case 0X00160050: return 668;
                case 0X00160051: return 669;
                case 0X00160061: return 670;
                case 0X00160062: return 671;
                case 0X00160070: return 672;
                case 0X00160071: return 673;
                case 0X00160072: return 674;
                case 0X00160073: return 675;
                case 0X00160074: return 676;
                case 0X00160075: return 677;
                case 0X00160076: return 678;
                case 0X00160077: return 679;
                case 0X00160078: return 680;
                case 0X00160079: return 681;
                case 0X0016007A: return 682;
                case 0X0016007B: return 683;
                case 0X0016007C: return 684;
                case 0X0016007D: return 685;
                case 0X0016007E: return 686;
                case 0X0016007F: return 687;
                case 0X00160080: return 688;
                case 0X00160081: return 689;
                case 0X00160082: return 690;
                case 0X00160083: return 691;
                case 0X00160084: return 692;
                case 0X00160085: return 693;
                case 0X00160086: return 694;
                case 0X00160087: return 695;
                case 0X00160088: return 696;
                case 0X00160089: return 697;
                case 0X0016008A: return 698;
                case 0X0016008B: return 699;
                case 0X0016008C: return 700;
                case 0X0016008D: return 701;
                case 0X0016008E: return 702;
                case 0X00161001: return 703;
                case 0X00161002: return 704;
                case 0X00161003: return 705;
                case 0X00161004: return 706;
                case 0X00161005: return 707;
                default:         return -1;
            }

        case 0X0018:
            switch (tag) {
                case 0X00180010: return 708;
                case 0X00180012: return 709;
                case 0X00180013: return 710;
                case 0X00180014: return 711;
                case 0X00180015: return 712;
                case 0X00180020: return 713;
                case 0X00180021: return 714;
                case 0X00180022: return 715;
                case 0X00180023: return 716;
                case 0X00180024: return 717;
                case 0X00180025: return 718;
                case 0X00180026: return 719;
                case 0X00180027: return 720;
                case 0X00180028: return 721;
                case 0X00180029: return 722;
                case 0X0018002A: return 723;
                case 0X00180030: return 724;
                case 0X00180031: return 725;
                case 0X00180032: return 726;
                case 0X00180033: return 727;
                case 0X00180034: return 728;
                case 0X00180035: return 729;
                case 0X00180036: return 730;
                case 0X00180037: return 731;
                case 0X00180038: return 732;
                case 0X00180039: return 733;
                case 0X0018003A: return 734;
                case 0X00180040: return 735;
                case 0X00180042: return 736;
                case 0X00180050: return 737;
                case 0X00180060: return 738;
                case 0X00180061: return 739;
                case 0X00180070: return 740;
                case 0X00180071: return 741;
                case 0X00180072: return 742;
                case 0X00180073: return 743;
                case 0X00180074: return 744;
                case 0X00180075: return 745;
                case 0X00180080: return 746;
                case 0X00180081: return 747;
                case 0X00180082: return 748;
                case 0X00180083: return 749;
                case 0X00180084: return 750;
                case 0X00180085: return 751;
                case 0X00180086: return 752;
                case 0X00180087: return 753;
                case 0X00180088: return 754;
                case 0X00180089: return 755;
                case 0X00180090: return 756;
                case 0X00180091: return 757;
                case 0X00180093: return 758;
                case 0X00180094: return 759;
                case 0X00180095: return 760;
                case 0X00181000: return 761;
                case 0X00181002: return 762;
                case 0X00181003: return 763;
                case 0X00181004: return 764;
                case 0X00181005: return 765;
                case 0X00181006: return 766;
                case 0X00181007: return 767;
                case 0X00181008: return 768;
                case 0X00181009: return 769;
                case 0X0018100A: return 770;
                case 0X0018100B: return 771;
                case 0X00181010: return 772;
                case 0X00181011: return 773;
                case 0X00181012: return 774;
                case 0X00181014: return 775;
                case 0X00181016: return 776;
                case 0X00181017: return 777;
                case 0X00181018: return 778;
                case 0X00181019: return 779;
                case 0X0018101A: return 780;
                case 0X0018101B: return 781;
                case 0X00181020: return 782;
                case 0X00181022: return 783;
                case 0X00181023: return 784;
                case 0X00181030: return 785;
                case 0X00181040: return 786;
                case 0X00181041: return 787;
                case 0X00181042: return 788;
                case 0X00181043: return 789;
                case 0X00181044: return 790;
                case 0X00181045: return 791;
                case 0X00181046: return 792;
                case 0X00181047: return 793;
                case 0X00181048: return 794;
                case 0X00181049: return 795;
                case 0X00181050: return 796;
                case 0X00181060: return 797;
                case 0X00181061: return 798;
                case 0X00181062: return 799;
                case 0X00181063: return 800;
                case 0X00181064: return 801;
                case 0X00181065: return 802;
                case 0X00181066: return 803;
                case 0X00181067: return 804;
                case 0X00181068: return 805;
                case 0X00181069: return 806;
                case 0X0018106A: return 807;
                case 0X0018106C: return 808;
                case 0X0018106E: return 809;
                case 0X00181070: return 810;
                case 0X00181071: return 811;
                case 0X00181072: return 812;
                case 0X00181073: return 813;
                case 0X00181074: return 814;
                case 0X00181075: return 815;
                case 0X00181076: return 816;
                case 0X00181077: return 817;
                case 0X00181078: return 818;
                case 0X00181079: return 819;
                case 0X00181080: return 820;
                case 0X00181081: return 821;
                case 0X00181082: return 822;
                case 0X00181083: return 823;
                case 0X00181084: return 824;
                case 0X00181085: return 825;
                case 0X00181086: return 826;
                case 0X00181088: return 827;
                case 0X00181090: return 828;
                case 0X00181094: return 829;
                case 0X00181100: return 830;
                case 0X00181110: return 831;
                case 0X00181111: return 832;
                case 0X00181114: return 833;
                case 0X00181120: return 834;
                case 0X00181121: return 835;
                case 0X00181130: return 836;
                case 0X00181131: return 837;
                case 0X00181134: return 838;
                case 0X00181135: return 839;
                case 0X00181136: return 840;
                case 0X00181137: return 841;
                case 0X00181138: return 842;
                case 0X0018113A: return 843;
                case 0X00181140: return 844;
                case 0X00181141: return 845;
                case 0X00181142: return 846;
                case 0X00181143: return 847;
                case 0X00181144: return 848;
                case 0X00181145: return 849;
                case 0X00181146: return 850;
                case 0X00181147: return 851;
                case 0X00181149: return 852;
                case 0X00181150: return 853;
                case 0X00181151: return 854;
                case 0X00181152: return 855;
                case 0X00181153: return 856;
                case 0X00181154: return 857;
                case 0X00181155: return 858;
                case 0X00181156: return 859;
                case 0X0018115A: return 860;
                case 0X0018115E: return 861;
                case 0X00181160: return 862;
                case 0X00181161: return 863;
                case 0X00181162: return 864;
                case 0X00181164: return 865;
                case 0X00181166: return 866;
                case 0X00181170: return 867;
                case 0X00181180: return 868;
                case 0X00181181: return 869;
                case 0X00181182: return 870;
                case 0X00181183: return 871;
                case 0X00181184: return 872;
                case 0X00181190: return 873;
                case 0X00181191: return 874;
                case 0X001811A0: return 875;
                case 0X001811A2: return 876;
                case 0X001811A3: return 877;
                case 0X001811A4: return 878;
                case 0X001811A5: return 879;
                case 0X001811B0: return 880;
                case 0X001811B1: return 881;
                case 0X001811B2: return 882;
                case 0X001811B3: return 883;
                case 0X001811B4: return 884;
                case 0X001811B5: return 885;
                case 0X001811B6: return 886;
                case 0X001811B7: return 887;
                case 0X001811B8: return 888;
                case 0X001811B9: return 889;
                case 0X001811BA: return 890;
                case 0X001811BB: return 891;
                case 0X001811BC: return 892;
                case 0X001811BD: return 893;
                case 0X001811BE: return 894;
                case 0X001811BF: return 895;
                case 0X001811C0: return 896;
                case 0X001811C1: return 897;
                case 0X00181200: return 898;
                case 0X00181201: return 899;
                case 0X00181202: return 900;
                case 0X00181210: return 901;
                case 0X00181240: return 902;
                case 0X00181242: return 903;
                case 0X00181243: return 904;
                case 0X00181244: return 905;
                case 0X00181250: return 906;
                case 0X00181251: return 907;
                case 0X00181260: return 908;
                case 0X00181261: return 909;
                case 0X00181271: return 910;
                case 0X00181272: return 911;
                case 0X00181300: return 912;
                case 0X00181301: return 913;
                case 0X00181302: return 914;
                case 0X00181310: return 915;
                case 0X00181312: return 916;
                case 0X00181314: return 917;
                case 0X00181315: return 918;
                case 0X00181316: return 919;
                case 0X00181318: return 920;
                case 0X00181320: return 921;
                case 0X00181400: return 922;
                case 0X00181401: return 923;
                case 0X00181402: return 924;
                case 0X00181403: return 925;
                case 0X00181404: return 926;
                case 0X00181405: return 927;
                case 0X00181411: return 928;
                case 0X00181412: return 929;
                case 0X00181413: return 930;
                case 0X00181450: return 931;
                case 0X00181460: return 932;
                case 0X00181470: return 933;
                case 0X00181480: return 934;
                case 0X00181490: return 935;
                case 0X00181491: return 936;
                case 0X00181495: return 937;
                case 0X00181500: return 938;
                case 0X00181508: return 939;
                case 0X00181510: return 940;
                case 0X00181511: return 941;
                case 0X00181520: return 942;
                case 0X00181521: return 943;
                case 0X00181530: return 944;
                case 0X00181531: return 945;
                case 0X00181600: return 946;
                case 0X00181602: return 947;
                case 0X00181604: return 948;
                case 0X00181606: return 949;
                case 0X00181608: return 950;
                case 0X00181610: return 951;
                case 0X00181612: return 952;
                case 0X00181620: return 953;
                case 0X00181622: return 954;
                case 0X00181623: return 955;
                case 0X00181624: return 956;
                case 0X00181630: return 957;
                case 0X00181631: return 958;
                case 0X00181632: return 959;
                case 0X00181633: return 960;
                case 0X00181634: return 961;
                case 0X00181635: return 962;
                case 0X00181636: return 963;
                case 0X00181637: return 964;
                case 0X00181638: return 965;
                case 0X00181700: return 966;
                case 0X00181702: return 967;
                case 0X00181704: return 968;
                case 0X00181706: return 969;
                case 0X00181708: return 970;
                case 0X00181710: return 971;
                case 0X00181712: return 972;
                case 0X00181720: return 973;
                case 0X00181800: return 974;
                case 0X00181801: return 975;
                case 0X00181802: return 976;
                case 0X00181803: return 977;
                case 0X00182001: return 978;
                case 0X00182002: return 979;
                case 0X00182003: return 980;
                case 0X00182004: return 981;
                case 0X00182005: return 982;
                case 0X00182006: return 983;
                case 0X00182010: return 984;
                case 0X00182020: return 985;
                case 0X00182030: return 986;
                case 0X00182041: return 987;
                case 0X00182042: return 988;
                case 0X00182043: return 989;
                case 0X00182044: return 990;
                case 0X00182045: return 991;
                case 0X00182046: return 992;
                case 0X00183100: return 993;
                case 0X00183101: return 994;
                case 0X00183102: return 995;
                case 0X00183103: return 996;
                case 0X00183104: return 997;
                case 0X00183105: return 998;
                case 0X00184000: return 999;
                case 0X00185000: return 1000;
                case 0X00185010: return 1001;
                case 0X00185011: return 1002;
                case 0X00185012: return 1003;
                case 0X00185020: return 1004;
                case 0X00185021: return 1005;
                case 0X00185022: return 1006;
                case 0X00185024: return 1007;
                case 0X00185026: return 1008;
                case 0X00185027: return 1009;
                case 0X00185028: return 1010;
                case 0X00185029: return 1011;
                case 0X00185030: return 1012;
                case 0X00185040: return 1013;
                case 0X00185050: return 1014;
                case 0X00185100: return 1015;
                case 0X00185101: return 1016;
                case 0X00185104: return 1017;
                case 0X00185210: return 1018;
                case 0X00185212: return 1019;
                case 0X00186000: return 1020;
                case 0X00186011: return 1021;
                case 0X00186012: return 1022;
                case 0X00186014: return 1023;
                case 0X00186016: return 1024;
                case 0X00186018: return 1025;
                case 0X0018601A: return 1026;
                case 0X0018601C: return 1027;
                case 0X0018601E: return 1028;
                case 0X00186020: return 1029;
                case 0X00186022: return 1030;
                case 0X00186024: return 1031;
                case 0X00186026: return 1032;
                case 0X00186028: return 1033;
                case 0X0018602A: return 1034;
                case 0X0018602C: return 1035;
                case 0X0018602E: return 1036;
                case 0X00186030: return 1037;
                case 0X00186031: return 1038;
                case 0X00186032: return 1039;
                case 0X00186034: return 1040;
                case 0X00186036: return 1041;
                case 0X00186038: return 1042;
                case 0X00186039: return 1043;
                case 0X0018603A: return 1044;
                case 0X0018603B: return 1045;
                case 0X0018603C: return 1046;
                case 0X0018603D: return 1047;
                case 0X0018603E: return 1048;
                case 0X0018603F: return 1049;
                case 0X00186040: return 1050;
                case 0X00186041: return 1051;
                case 0X00186042: return 1052;
                case 0X00186043: return 1053;
                case 0X00186044: return 1054;
                case 0X00186046: return 1055;
                case 0X00186048: return 1056;
                case 0X0018604A: return 1057;
                case 0X0018604C: return 1058;
                case 0X0018604E: return 1059;
                case 0X00186050: return 1060;
                case 0X00186052: return 1061;
                case 0X00186054: return 1062;
                case 0X00186056: return 1063;
                case 0X00186058: return 1064;
                case 0X0018605A: return 1065;
                case 0X00186060: return 1066;
                case 0X00186070: return 1067;
                case 0X00187000: return 1068;
                case 0X00187001: return 1069;
                case 0X00187004: return 1070;
                case 0X00187005: return 1071;
                case 0X00187006: return 1072;
                case 0X00187008: return 1073;
                case 0X0018700A: return 1074;
                case 0X0018700C: return 1075;
                case 0X0018700E: return 1076;
                case 0X00187010: return 1077;
                case 0X00187011: return 1078;
                case 0X00187012: return 1079;
                case 0X00187014: return 1080;
                case 0X00187016: return 1081;
                case 0X0018701A: return 1082;
                case 0X00187020: return 1083;
                case 0X00187022: return 1084;
                case 0X00187024: return 1085;
                case 0X00187026: return 1086;
                case 0X00187028: return 1087;
                case 0X0018702A: return 1088;
                case 0X0018702B: return 1089;
                case 0X00187030: return 1090;
                case 0X00187032: return 1091;
                case 0X00187034: return 1092;
                case 0X00187036: return 1093;
                case 0X00187038: return 1094;
                case 0X00187040: return 1095;
                case 0X00187041: return 1096;
                case 0X00187042: return 1097;
                case 0X00187044: return 1098;
                case 0X00187046: return 1099;
                case 0X00187048: return 1100;
                case 0X0018704C: return 1101;
                case 0X00187050: return 1102;
                case 0X00187052: return 1103;
                case 0X00187054: return 1104;
                case 0X00187056: return 1105;
                case 0X00187058: return 1106;
                case 0X00187060: return 1107;
                case 0X00187062: return 1108;
                case 0X00187064: return 1109;
                case 0X00187065: return 1110;
                case 0X00188150: return 1111;
                case 0X00188151: return 1112;
                case 0X00189004: return 1113;
                case 0X00189005: return 1114;
                case 0X00189006: return 1115;
                case 0X00189008: return 1116;
                case 0X00189009: return 1117;
                case 0X00189010: return 1118;
                case 0X00189011: return 1119;
                case 0X00189012: return 1120;
                case 0X00189014: return 1121;
                case 0X00189015: return 1122;
                case 0X00189016: return 1123;
                case 0X00189017: return 1124;
                case 0X00189018: return 1125;
                case 0X00189019: return 1126;
                case 0X00189020: return 1127;
                case 0X00189021: return 1128;
                case 0X00189022: return 1129;
                case 0X00189024: return 1130;
                case 0X00189025: return 1131;
                case 0X00189026: return 1132;
                case 0X00189027: return 1133;
                case 0X00189028: return 1134;
                case 0X00189029: return 1135;
                case 0X00189030: return 1136;
                case 0X00189032: return 1137;
                case 0X00189033: return 1138;
                case 0X00189034: return 1139;
                case 0X00189035: return 1140;
                case 0X00189036: return 1141;
                case 0X00189037: return 1142;
                case 0X00189041: return 1143;
                case 0X00189042: return 1144;
                case 0X00189043: return 1145;
                case 0X00189044: return 1146;
                case 0X00189045: return 1147;
                case 0X00189046: return 1148;
                case 0X00189047: return 1149;
                case 0X00189048: return 1150;
                case 0X00189049: return 1151;
                case 0X00189050: return 1152;
                case 0X00189051: return 1153;
                case 0X00189052: return 1154;
                case 0X00189053: return 1155;
                case 0X00189054: return 1156;
                case 0X00189058: return 1157;
                case 0X00189059: return 1158;
                case 0X00189060: return 1159;
                case 0X00189061: return 1160;
                case 0X00189062: return 1161;
                case 0X00189063: return 1162;
                case 0X00189064: return 1163;
                case 0X00189065: return 1164;
                case 0X00189066: return 1165;
                case 0X00189067: return 1166;
                case 0X00189069: return 1167;
                case 0X00189070: return 1168;
                case 0X00189073: return 1169;
                case 0X00189074: return 1170;
                case 0X00189075: return 1171;
                case 0X00189076: return 1172;
                case 0X00189077: return 1173;
                case 0X00189078: return 1174;
                case 0X00189079: return 1175;
                case 0X00189080: return 1176;
                case 0X00189081: return 1177;
                case 0X00189082: return 1178;
                case 0X00189083: return 1179;
                case 0X00189084: return 1180;
                case 0X00189085: return 1181;
                case 0X00189087: return 1182;
                case 0X00189089: return 1183;
                case 0X00189090: return 1184;
                case 0X00189091: return 1185;
                case 0X00189092: return 1186;
                case 0X00189093: return 1187;
                case 0X00189094: return 1188;
                case 0X00189095: return 1189;
                case 0X00189096: return 1190;
                case 0X00189098: return 1191;
                case 0X00189100: return 1192;
                case 0X00189101: return 1193;
                case 0X00189103: return 1194;
                case 0X00189104: return 1195;
                case 0X00189105: return 1196;
                case 0X00189106: return 1197;
                case 0X00189107: return 1198;
                case 0X00189112: return 1199;
                case 0X00189114: return 1200;
                case 0X00189115: return 1201;
                case 0X00189117: return 1202;
                case 0X00189118: return 1203;
                case 0X00189119: return 1204;
                case 0X00189125: return 1205;
                case 0X00189126: return 1206;
                case 0X00189127: return 1207;
                case 0X00189147: return 1208;
                case 0X00189151: return 1209;
                case 0X00189152: return 1210;
                case 0X00189155: return 1211;
                case 0X00189159: return 1212;
                case 0X00189166: return 1213;
                case 0X00189168: return 1214;
                case 0X00189169: return 1215;
                case 0X00189170: return 1216;
                case 0X00189171: return 1217;
                case 0X00189172: return 1218;
                case 0X00189173: return 1219;
                case 0X00189174: return 1220;
                case 0X00189175: return 1221;
                case 0X00189176: return 1222;
                case 0X00189177: return 1223;
                case 0X00189178: return 1224;
                case 0X00189179: return 1225;
                case 0X00189180: return 1226;
                case 0X00189181: return 1227;
                case 0X00189182: return 1228;
                case 0X00189183: return 1229;
                case 0X00189184: return 1230;
                case 0X00189185: return 1231;
                case 0X00189186: return 1232;
                case 0X00189195: return 1233;
                case 0X00189196: return 1234;
                case 0X00189197: return 1235;
                case 0X00189198: return 1236;
                case 0X00189199: return 1237;
                case 0X00189200: return 1238;
                case 0X00189214: return 1239;
                case 0X00189217: return 1240;
                case 0X00189218: return 1241;
                case 0X00189219: return 1242;
                case 0X00189220: return 1243;
                case 0X00189226: return 1244;
                case 0X00189227: return 1245;
                case 0X00189231: return 1246;
                case 0X00189232: return 1247;
                case 0X00189234: return 1248;
                case 0X00189236: return 1249;
                case 0X00189239: return 1250;
                case 0X00189240: return 1251;
                case 0X00189241: return 1252;
                case 0X00189250: return 1253;
                case 0X00189251: return 1254;
                case 0X00189252: return 1255;
                case 0X00189253: return 1256;
                case 0X00189254: return 1257;
                case 0X00189255: return 1258;
                case 0X00189256: return 1259;
                case 0X00189257: return 1260;
                case 0X00189258: return 1261;
                case 0X00189259: return 1262;
                case 0X0018925A: return 1263;
                case 0X0018925B: return 1264;
                case 0X0018925C: return 1265;
                case 0X0018925D: return 1266;
                case 0X0018925E: return 1267;
                case 0X0018925F: return 1268;
                case 0X00189260: return 1269;
                case 0X00189295: return 1270;
                case 0X00189296: return 1271;
                case 0X00189297: return 1272;
                case 0X00189298: return 1273;
                case 0X00189301: return 1274;
                case 0X00189302: return 1275;
                case 0X00189303: return 1276;
                case 0X00189304: return 1277;
                case 0X00189305: return 1278;
                case 0X00189306: return 1279;
                case 0X00189307: return 1280;
                case 0X00189308: return 1281;
                case 0X00189309: return 1282;
                case 0X00189310: return 1283;
                case 0X00189311: return 1284;
                case 0X00189312: return 1285;
                case 0X00189313: return 1286;
                case 0X00189314: return 1287;
                case 0X00189315: return 1288;
                case 0X00189316: return 1289;
                case 0X00189317: return 1290;
                case 0X00189318: return 1291;
                case 0X00189319: return 1292;
                case 0X00189320: return 1293;
                case 0X00189321: return 1294;
                case 0X00189322: return 1295;
                case 0X00189323: return 1296;
                case 0X00189324: return 1297;
                case 0X00189325: return 1298;
                case 0X00189326: return 1299;
                case 0X00189327: return 1300;
                case 0X00189328: return 1301;
                case 0X00189329: return 1302;
                case 0X00189330: return 1303;
                case 0X00189332: return 1304;
                case 0X00189333: return 1305;
                case 0X00189334: return 1306;
                case 0X00189335: return 1307;
                case 0X00189337: return 1308;
                case 0X00189338: return 1309;
                case 0X00189340: return 1310;
                case 0X00189341: return 1311;
                case 0X00189342: return 1312;
                case 0X00189343: return 1313;
                case 0X00189344: return 1314;
                case 0X00189345: return 1315;
                case 0X00189346: return 1316;
                case 0X00189351: return 1317;
                case 0X00189352: return 1318;
                case 0X00189353: return 1319;
                case 0X00189360: return 1320;
                case 0X00189361: return 1321;
                case 0X00189362: return 1322;
                case 0X00189363: return 1323;
                case 0X00189364: return 1324;
                case 0X00189365: return 1325;
                case 0X00189366: return 1326;
                case 0X00189367: return 1327;
                case 0X00189368: return 1328;
                case 0X00189369: return 1329;
                case 0X0018936A: return 1330;
                case 0X0018936B: return 1331;
                case 0X0018936C: return 1332;
                case 0X0018936D: return 1333;
                case 0X0018936E: return 1334;
                case 0X0018936F: return 1335;
                case 0X00189370: return 1336;
                case 0X00189371: return 1337;
                case 0X00189372: return 1338;
                case 0X00189373: return 1339;
                case 0X00189374: return 1340;
                case 0X00189375: return 1341;
                case 0X00189376: return 1342;
                case 0X00189377: return 1343;
                case 0X00189378: return 1344;
                case 0X00189379: return 1345;
                case 0X0018937A: return 1346;
                case 0X0018937B: return 1347;
                case 0X0018937C: return 1348;
                case 0X0018937D: return 1349;
                case 0X0018937E: return 1350;
                case 0X0018937F: return 1351;
                case 0X00189380: return 1352;
                case 0X00189381: return 1353;
                case 0X00189382: return 1354;
                case 0X00189383: return 1355;
                case 0X00189384: return 1356;
                case 0X00189401: return 1357;
                case 0X00189402: return 1358;
                case 0X00189403: return 1359;
                case 0X00189404: return 1360;
                case 0X00189405: return 1361;
                case 0X00189406: return 1362;
                case 0X00189407: return 1363;
                case 0X00189410: return 1364;
                case 0X00189412: return 1365;
                case 0X00189417: return 1366;
                case 0X00189420: return 1367;
                case 0X00189423: return 1368;
                case 0X00189424: return 1369;
                case 0X00189425: return 1370;
                case 0X00189426: return 1371;
                case 0X00189427: return 1372;
                case 0X00189428: return 1373;
                case 0X00189429: return 1374;
                case 0X00189430: return 1375;
                case 0X00189432: return 1376;
                case 0X00189433: return 1377;
                case 0X00189434: return 1378;
                case 0X00189435: return 1379;
                case 0X00189436: return 1380;
                case 0X00189437: return 1381;
                case 0X00189438: return 1382;
                case 0X00189439: return 1383;
                case 0X00189440: return 1384;
                case 0X00189441: return 1385;
                case 0X00189442: return 1386;
                case 0X00189445: return 1387;
                case 0X00189447: return 1388;
                case 0X00189449: return 1389;
                case 0X00189451: return 1390;
                case 0X00189452: return 1391;
                case 0X00189455: return 1392;
                case 0X00189456: return 1393;
                case 0X00189457: return 1394;
                case 0X00189461: return 1395;
                case 0X00189462: return 1396;
                case 0X00189463: return 1397;
                case 0X00189464: return 1398;
                case 0X00189465: return 1399;
                case 0X00189466: return 1400;
                case 0X00189467: return 1401;
                case 0X00189468: return 1402;
                case 0X00189469: return 1403;
                case 0X00189470: return 1404;
                case 0X00189471: return 1405;
                case 0X00189472: return 1406;
                case 0X00189473: return 1407;
                case 0X00189474: return 1408;
                case 0X00189476: return 1409;
                case 0X00189477: return 1410;
                case 0X00189504: return 1411;
                case 0X00189506: return 1412;
                case 0X00189507: return 1413;
                case 0X00189508: return 1414;
                case 0X00189509: return 1415;
                case 0X00189510: return 1416;
                case 0X00189511: return 1417;
                case 0X00189514: return 1418;
                case 0X00189515: return 1419;
                case 0X00189516: return 1420;
                case 0X00189517: return 1421;
                case 0X00189518: return 1422;
                case 0X00189519: return 1423;
                case 0X00189524: return 1424;
                case 0X00189525: return 1425;
                case 0X00189526: return 1426;
                case 0X00189527: return 1427;
                case 0X00189528: return 1428;
                case 0X00189530: return 1429;
                case 0X00189531: return 1430;
                case 0X00189538: return 1431;
                case 0X00189541: return 1432;
                case 0X00189542: return 1433;
                case 0X00189543: return 1434;
                case 0X00189544: return 1435;
                case 0X00189545: return 1436;
                case 0X00189546: return 1437;
                case 0X00189547: return 1438;
                case 0X00189548: return 1439;
                case 0X00189549: return 1440;
                case 0X00189550: return 1441;
                case 0X00189551: return 1442;
                case 0X00189552: return 1443;
                case 0X00189553: return 1444;
                case 0X00189554: return 1445;
                case 0X00189555: return 1446;
                case 0X00189556: return 1447;
                case 0X00189557: return 1448;
                case 0X00189558: return 1449;
                case 0X00189559: return 1450;
                case 0X00189601: return 1451;
                case 0X00189602: return 1452;
                case 0X00189603: return 1453;
                case 0X00189604: return 1454;
                case 0X00189605: return 1455;
                case 0X00189606: return 1456;
                case 0X00189607: return 1457;
                case 0X00189621: return 1458;
                case 0X00189622: return 1459;
                case 0X00189623: return 1460;
                case 0X00189624: return 1461;
                case 0X00189701: return 1462;
                case 0X00189715: return 1463;
                case 0X00189716: return 1464;
                case 0X00189717: return 1465;
                case 0X00189718: return 1466;
                case 0X00189719: return 1467;
                case 0X00189720: return 1468;
                case 0X00189721: return 1469;
                case 0X00189722: return 1470;
                case 0X00189723: return 1471;
                case 0X00189724: return 1472;
                case 0X00189725: return 1473;
                case 0X00189726: return 1474;
                case 0X00189727: return 1475;
                case 0X00189729: return 1476;
                case 0X00189732: return 1477;
                case 0X00189733: return 1478;
                case 0X00189734: return 1479;
                case 0X00189735: return 1480;
                case 0X00189736: return 1481;
                case 0X00189737: return 1482;
                case 0X00189738: return 1483;
                case 0X00189739: return 1484;
                case 0X00189740: return 1485;
                case 0X00189749: return 1486;
                case 0X00189751: return 1487;
                case 0X00189755: return 1488;
                case 0X00189756: return 1489;
                case 0X00189758: return 1490;
                case 0X00189759: return 1491;
                case 0X00189760: return 1492;
                case 0X00189761: return 1493;
                case 0X00189762: return 1494;
                case 0X00189763: return 1495;
                case 0X00189764: return 1496;
                case 0X00189765: return 1497;
                case 0X00189766: return 1498;
                case 0X00189767: return 1499;
                case 0X00189768: return 1500;
                case 0X00189769: return 1501;
                case 0X00189770: return 1502;
                case 0X00189771: return 1503;
                case 0X00189772: return 1504;
                case 0X00189801: return 1505;
                case 0X00189803: return 1506;
                case 0X00189804: return 1507;
                case 0X00189805: return 1508;
                case 0X00189806: return 1509;
                case 0X00189807: return 1510;
                case 0X00189808: return 1511;
                case 0X00189809: return 1512;
                case 0X0018980B: return 1513;
                case 0X0018980C: return 1514;
                case 0X0018980D: return 1515;
                case 0X0018980E: return 1516;
                case 0X0018980F: return 1517;
                case 0X00189810: return 1518;
                case 0X00189900: return 1519;
                case 0X00189901: return 1520;
                case 0X00189902: return 1521;
                case 0X00189903: return 1522;
                case 0X00189904: return 1523;
                case 0X00189905: return 1524;
                case 0X00189906: return 1525;
                case 0X00189907: return 1526;
                case 0X00189908: return 1527;
                case 0X00189909: return 1528;
                case 0X0018990A: return 1529;
                case 0X0018990B: return 1530;
                case 0X0018990C: return 1531;
                case 0X0018990D: return 1532;
                case 0X0018990E: return 1533;
                case 0X0018990F: return 1534;
                case 0X00189910: return 1535;
                case 0X00189911: return 1536;
                case 0X00189912: return 1537;
                case 0X00189913: return 1538;
                case 0X00189914: return 1539;
                case 0X00189915: return 1540;
                case 0X00189916: return 1541;
                case 0X00189917: return 1542;
                case 0X00189918: return 1543;
                case 0X00189919: return 1544;
                case 0X0018991A: return 1545;
                case 0X0018991B: return 1546;
                case 0X0018991C: return 1547;
                case 0X0018991D: return 1548;
                case 0X0018991E: return 1549;
                case 0X0018991F: return 1550;
                case 0X00189920: return 1551;
                case 0X00189921: return 1552;
                case 0X00189922: return 1553;
                case 0X00189923: return 1554;
                case 0X00189924: return 1555;
                case 0X00189930: return 1556;
                case 0X00189931: return 1557;
                case 0X00189932: return 1558;
                case 0X00189933: return 1559;
                case 0X00189934: return 1560;
                case 0X00189935: return 1561;
                case 0X00189936: return 1562;
                case 0X00189937: return 1563;
                case 0X00189938: return 1564;
                case 0X00189939: return 1565;
                case 0X0018993A: return 1566;
                case 0X0018993B: return 1567;
                case 0X0018993C: return 1568;
                case 0X0018993D: return 1569;
                case 0X0018993E: return 1570;
                case 0X00189941: return 1571;
                case 0X00189942: return 1572;
                case 0X00189943: return 1573;
                case 0X00189944: return 1574;
                case 0X00189945: return 1575;
                case 0X00189946: return 1576;
                case 0X00189947: return 1577;
                case 0X0018A001: return 1578;
                case 0X0018A002: return 1579;
                case 0X0018A003: return 1580;
                default:         return -1;
            }

        case 0X0020:
            switch (tag) {
                case 0X0020000D: return 1581;
                case 0X0020000E: return 1582;
                case 0X00200010: return 1583;
                case 0X00200011: return 1584;
                case 0X00200012: return 1585;
                case 0X00200013: return 1586;
                case 0X00200014: return 1587;
                case 0X00200015: return 1588;
                case 0X00200016: return 1589;
                case 0X00200017: return 1590;
                case 0X00200018: return 1591;
                case 0X00200019: return 1592;
                case 0X00200020: return 1593;
                case 0X00200022: return 1594;
                case 0X00200024: return 1595;
                case 0X00200026: return 1596;
                case 0X00200030: return 1597;
                case 0X00200032: return 1598;
                case 0X00200035: return 1599;
                case 0X00200037: return 1600;
                case 0X00200050: return 1601;
                case 0X00200052: return 1602;
                case 0X00200060: return 1603;
                case 0X00200062: return 1604;
                case 0X00200070: return 1605;
                case 0X00200080: return 1606;
                case 0X002000AA: return 1607;
                case 0X00200100: return 1608;
                case 0X00200105: return 1609;
                case 0X00200110: return 1610;
                case 0X00200200: return 1611;
                case 0X00200242: return 1612;
                case 0X00201000: return 1613;
                case 0X00201001: return 1614;
                case 0X00201002: return 1615;
                case 0X00201003: return 1616;
                case 0X00201004: return 1617;
                case 0X00201005: return 1618;
                case 0X00201020: return 1619;
                case 0X0020103F: return 1620;
                case 0X00201040: return 1621;
                case 0X00201041: return 1622;
                case 0X00201070: return 1623;
                case 0X00201200: return 1624;
                case 0X00201202: return 1625;
                case 0X00201204: return 1626;
                case 0X00201206: return 1627;
                case 0X00201208: return 1628;
                case 0X00201209: return 1629;
                case 0X00203401: return 1630;
                case 0X00203402: return 1631;
                case 0X00203403: return 1632;
                case 0X00203404: return 1633;
                case 0X00203405: return 1634;
                case 0X00203406: return 1635;
                case 0X00204000: return 1636;
                case 0X00205000: return 1637;
                case 0X00205002: return 1638;
                case 0X00209056: return 1639;
                case 0X00209057: return 1640;
                case 0X00209071: return 1641;
                case 0X00209072: return 1642;
                case 0X00209111: return 1643;
                case 0X00209113: return 1644;
                case 0X00209116: return 1645;
                case 0X00209128: return 1646;
                case 0X00209153: return 1647;
                case 0X00209154: return 1648;
                case 0X00209155: return 1649;
                case 0X00209156: return 1650;
                case 0X00209157: return 1651;
                case 0X00209158: return 1652;
                case 0X00209161: return 1653;
                case 0X00209162: return 1654;
                case 0X00209163: return 1655;
                case 0X00209164: return 1656;
                case 0X00209165: return 1657;
                case 0X00209167: return 1658;
                case 0X00209170: return 1659;
                case 0X00209171: return 1660;
                case 0X00209172: return 1661;
                case 0X00209213: return 1662;
                case 0X00209221: return 1663;
                case 0X00209222: return 1664;
                case 0X00209228: return 1665;
                case 0X00209238: return 1666;
                case 0X00209241: return 1667;
                case 0X00209245: return 1668;
                case 0X00209246: return 1669;
                case 0X00209247: return 1670;
                case 0X00209248: return 1671;
                case 0X00209249: return 1672;
                case 0X00209250: return 1673;
                case 0X00209251: return 1674;
                case 0X00209252: return 1675;
                case 0X00209253: return 1676;
                case 0X00209254: return 1677;
                case 0X00209255: return 1678;
                case 0X00209256: return 1679;
                case 0X00209257: return 1680;
                case 0X00209301: return 1681;
                case 0X00209302: return 1682;
                case 0X00209307: return 1683;
                case 0X00209308: return 1684;
                case 0X00209309: return 1685;
                case 0X0020930A: return 1686;
                case 0X0020930B: return 1687;
                case 0X0020930C: return 1688;
                case 0X0020930D: return 1689;
                case 0X0020930E: return 1690;
                case 0X0020930F: return 1691;
                case 0X00209310: return 1692;
                case 0X00209311: return 1693;
                case 0X00209312: return 1694;
                case 0X00209313: return 1695;
                case 0X00209421: return 1696;
                case 0X00209450: return 1697;
                case 0X00209453: return 1698;
                case 0X00209518: return 1699;
                case 0X00209529: return 1700;
                case 0X00209536: return 1701;
                default:         return -1;
            }

        case 0X0022:
            switch (tag) {
                case 0X00220001: return 1702;
                case 0X00220002: return 1703;
                case 0X00220003: return 1704;
                case 0X00220004: return 1705;
                case 0X00220005: return 1706;
                case 0X00220006: return 1707;
                case 0X00220007: return 1708;
                case 0X00220008: return 1709;
                case 0X00220009: return 1710;
                case 0X0022000A: return 1711;
                case 0X0022000B: return 1712;
                case 0X0022000C: return 1713;
                case 0X0022000D: return 1714;
                case 0X0022000E: return 1715;
                case 0X00220010: return 1716;
                case 0X00220011: return 1717;
                case 0X00220012: return 1718;
                case 0X00220013: return 1719;
                case 0X00220014: return 1720;
                case 0X00220015: return 1721;
                case 0X00220016: return 1722;
                case 0X00220017: return 1723;
                case 0X00220018: return 1724;
                case 0X00220019: return 1725;
                case 0X0022001A: return 1726;
                case 0X0022001B: return 1727;
                case 0X0022001C: return 1728;
                case 0X0022001D: return 1729;
                case 0X0022001E: return 1730;
                case 0X00220020: return 1731;
                case 0X00220021: return 1732;
                case 0X00220022: return 1733;
                case 0X00220028: return 1734;
                case 0X00220030: return 1735;
                case 0X00220031: return 1736;
                case 0X00220032: return 1737;
                case 0X00220035: return 1738;
                case 0X00220036: return 1739;
                case 0X00220037: return 1740;
                case 0X00220038: return 1741;
                case 0X00220039: return 1742;
                case 0X00220041: return 1743;
                case 0X00220042: return 1744;
                case 0X00220048: return 1745;
                case 0X00220049: return 1746;
                case 0X0022004E: return 1747;
                case 0X00220055: return 1748;
                case 0X00220056: return 1749;
                case 0X00220057: return 1750;
                case 0X00220058: return 1751;
                case 0X00221007: return 1752;
                case 0X00221008: return 1753;
                case 0X00221009: return 1754;
                case 0X00221010: return 1755;
                case 0X00221012: return 1756;
                case 0X00221019: return 1757;
                case 0X00221024: return 1758;
                case 0X00221025: return 1759;
                case 0X00221028: return 1760;
                case 0X00221029: return 1761;
                case 0X00221033: return 1762;
                case 0X00221035: return 1763;
                case 0X00221036: return 1764;
                case 0X00221037: return 1765;
                case 0X00221039: return 1766;
                case 0X00221040: return 1767;
                case 0X00221044: return 1768;
                case 0X00221045: return 1769;
                case 0X00221046: return 1770;
                case 0X00221047: return 1771;
                case 0X00221048: return 1772;
                case 0X00221049: return 1773;
                case 0X0022104A: return 1774;
                case 0X0022104B: return 1775;
                case 0X00221050: return 1776;
                case 0X00221053: return 1777;
                case 0X00221054: return 1778;
                case 0X00221059: return 1779;
                case 0X00221065: return 1780;
                case 0X00221066: return 1781;
                case 0X00221090: return 1782;
                case 0X00221092: return 1783;
                case 0X00221093: return 1784;
                case 0X00221094: return 1785;
                case 0X00221095: return 1786;
                case 0X00221096: return 1787;
                case 0X00221097: return 1788;
                case 0X00221100: return 1789;
                case 0X00221101: return 1790;
                case 0X00221103: return 1791;
                case 0X00221121: return 1792;
                case 0X00221122: return 1793;
                case 0X00221125: return 1794;
                case 0X00221127: return 1795;
                case 0X00221128: return 1796;
                case 0X0022112A: return 1797;
                case 0X0022112B: return 1798;
                case 0X0022112C: return 1799;
                case 0X00221130: return 1800;
                case 0X00221131: return 1801;
                case 0X00221132: return 1802;
                case 0X00221133: return 1803;
                case 0X00221134: return 1804;
                case 0X00221135: return 1805;
                case 0X00221140: return 1806;
                case 0X00221150: return 1807;
                case 0X00221153: return 1808;
                case 0X00221155: return 1809;
                case 0X00221159: return 1810;
                case 0X00221210: return 1811;
                case 0X00221211: return 1812;
                case 0X00221212: return 1813;
                case 0X00221220: return 1814;
                case 0X00221225: return 1815;
                case 0X00221230: return 1816;
                case 0X00221250: return 1817;
                case 0X00221255: return 1818;
                case 0X00221257: return 1819;
                case 0X00221260: return 1820;
                case 0X00221262: return 1821;
                case 0X00221265: return 1822;
                case 0X00221273: return 1823;
                case 0X00221300: return 1824;
                case 0X00221310: return 1825;
                case 0X00221330: return 1826;
                case 0X00221415: return 1827;
                case 0X00221420: return 1828;
                case 0X00221423: return 1829;
                case 0X00221436: return 1830;
                case 0X00221443: return 1831;
                case 0X00221445: return 1832;
                case 0X00221450: return 1833;
                case 0X00221452: return 1834;
                case 0X00221454: return 1835;
                case 0X00221458: return 1836;
                case 0X00221460: return 1837;
                case 0X00221463: return 1838;
                case 0X00221465: return 1839;
                case 0X00221466: return 1840;
                case 0X00221467: return 1841;
                case 0X00221468: return 1842;
                case 0X00221470: return 1843;
                case 0X00221472: return 1844;
                case 0X00221512: return 1845;
                case 0X00221513: return 1846;
                case 0X00221515: return 1847;
                case 0X00221517: return 1848;
                case 0X00221518: return 1849;
                case 0X00221525: return 1850;
                case 0X00221526: return 1851;
                case 0X00221527: return 1852;
                case 0X00221528: return 1853;
                case 0X00221529: return 1854;
                case 0X00221530: return 1855;
                case 0X00221531: return 1856;
                case 0X00221612: return 1857;
                case 0X00221615: return 1858;
                case 0X00221616: return 1859;
                case 0X00221618: return 1860;
                case 0X00221620: return 1861;
                case 0X00221622: return 1862;
                case 0X00221624: return 1863;
                case 0X00221626: return 1864;
                case 0X00221628: return 1865;
                case 0X00221630: return 1866;
                case 0X00221640: return 1867;
                case 0X00221642: return 1868;
                case 0X00221643: return 1869;
                case 0X00221644: return 1870;
                case 0X00221645: return 1871;
                case 0X00221646: return 1872;
                case 0X00221649: return 1873;
                case 0X00221650: return 1874;
                case 0X00221658: return 1875;
                default:         return -1;
            }

        case 0X0024:
            switch (tag) {
                case 0X00240010: return 1876;
                case 0X00240011: return 1877;
                case 0X00240012: return 1878;
                case 0X00240016: return 1879;
                case 0X00240018: return 1880;
                case 0X00240020: return 1881;
                case 0X00240021: return 1882;
                case 0X00240024: return 1883;
                case 0X00240025: return 1884;
                case 0X00240028: return 1885;
                case 0X00240032: return 1886;
                case 0X00240033: return 1887;
                case 0X00240034: return 1888;
                case 0X00240035: return 1889;
                case 0X00240036: return 1890;
                case 0X00240037: return 1891;
                case 0X00240038: return 1892;
                case 0X00240039: return 1893;
                case 0X00240040: return 1894;
                case 0X00240042: return 1895;
                case 0X00240044: return 1896;
                case 0X00240045: return 1897;
                case 0X00240046: return 1898;
                case 0X00240048: return 1899;
                case 0X00240050: return 1900;
                case 0X00240051: return 1901;
                case 0X00240052: return 1902;
                case 0X00240053: return 1903;
                case 0X00240054: return 1904;
                case 0X00240055: return 1905;
                case 0X00240056: return 1906;
                case 0X00240057: return 1907;
                case 0X00240058: return 1908;
                case 0X00240059: return 1909;
                case 0X00240060: return 1910;
                case 0X00240061: return 1911;
                case 0X00240062: return 1912;
                case 0X00240063: return 1913;
                case 0X00240064: return 1914;
                case 0X00240065: return 1915;
                case 0X00240066: return 1916;
                case 0X00240067: return 1917;
                case 0X00240068: return 1918;
                case 0X00240069: return 1919;
                case 0X00240070: return 1920;
                case 0X00240071: return 1921;
                case 0X00240072: return 1922;
                case 0X00240073: return 1923;
                case 0X00240074: return 1924;
                case 0X00240075: return 1925;
                case 0X00240076: return 1926;
                case 0X00240077: return 1927;
                case 0X00240078: return 1928;
                case 0X00240079: return 1929;
                case 0X00240080: return 1930;
                case 0X00240081: return 1931;
                case 0X00240083: return 1932;
                case 0X00240085: return 1933;
                case 0X00240086: return 1934;
                case 0X00240087: return 1935;
                case 0X00240088: return 1936;
                case 0X00240089: return 1937;
                case 0X00240090: return 1938;
                case 0X00240091: return 1939;
                case 0X00240092: return 1940;
                case 0X00240093: return 1941;
                case 0X00240094: return 1942;
                case 0X00240095: return 1943;
                case 0X00240096: return 1944;
                case 0X00240097: return 1945;
                case 0X00240098: return 1946;
                case 0X00240100: return 1947;
                case 0X00240102: return 1948;
                case 0X00240103: return 1949;
                case 0X00240104: return 1950;
                case 0X00240105: return 1951;
                case 0X00240106: return 1952;
                case 0X00240107: return 1953;
                case 0X00240108: return 1954;
                case 0X00240110: return 1955;
                case 0X00240112: return 1956;
                case 0X00240113: return 1957;
                case 0X00240114: return 1958;
                case 0X00240115: return 1959;
                case 0X00240117: return 1960;
                case 0X00240118: return 1961;
                case 0X00240120: return 1962;
                case 0X00240122: return 1963;
                case 0X00240124: return 1964;
                case 0X00240126: return 1965;
                case 0X00240202: return 1966;
                case 0X00240306: return 1967;
                case 0X00240307: return 1968;
                case 0X00240308: return 1969;
                case 0X00240309: return 1970;
                case 0X00240317: return 1971;
                case 0X00240320: return 1972;
                case 0X00240325: return 1973;
                case 0X00240338: return 1974;
                case 0X00240341: return 1975;
                case 0X00240344: return 1976;
                default:         return -1;
            }

        case 0X0028:
            switch (tag) {
                case 0X00280002: return 1977;
                case 0X00280003: return 1978;
                case 0X00280004: return 1979;
                case 0X00280005: return 1980;
                case 0X00280006: return 1981;
                case 0X00280008: return 1982;
                case 0X00280009: return 1983;
                case 0X0028000A: return 1984;
                case 0X00280010: return 1985;
                case 0X00280011: return 1986;
                case 0X00280012: return 1987;
                case 0X00280014: return 1988;
                case 0X00280020: return 1989;
                case 0X00280030: return 1990;
                case 0X00280031: return 1991;
                case 0X00280032: return 1992;
                case 0X00280034: return 1993;
                case 0X00280040: return 1994;
                case 0X00280050: return 1995;
                case 0X00280051: return 1996;
                case 0X0028005F: return 1997;
                case 0X00280060: return 1998;
                case 0X00280061: return 1999;
                case 0X00280062: return 2000;
                case 0X00280063: return 2001;
                case 0X00280065: return 2002;
                case 0X00280066: return 2003;
                case 0X00280068: return 2004;
                case 0X00280069: return 2005;
                case 0X00280070: return 2006;
                case 0X00280071: return 2007;
                case 0X00280080: return 2008;
                case 0X00280081: return 2009;
                case 0X00280082: return 2010;
                case 0X00280090: return 2011;
                case 0X00280091: return 2012;
                case 0X00280092: return 2013;
                case 0X00280093: return 2014;
                case 0X00280094: return 2015;
                case 0X00280100: return 2016;
                case 0X00280101: return 2017;
                case 0X00280102: return 2018;
                case 0X00280103: return 2019;
                case 0X00280104: return 2020;
                case 0X00280105: return 2021;
                case 0X00280106: return 2022;
                case 0X00280107: return 2023;
                case 0X00280108: return 2024;
                case 0X00280109: return 2025;
                case 0X00280110: return 2026;
                case 0X00280111: return 2027;
                case 0X00280120: return 2028;
                case 0X00280121: return 2029;
                case 0X00280122: return 2030;
                case 0X00280123: return 2031;
                case 0X00280124: return 2032;
                case 0X00280125: return 2033;
                case 0X00280200: return 2034;
                case 0X00280300: return 2035;
                case 0X00280301: return 2036;
                case 0X00280302: return 2037;
                case 0X00280303: return 2038;
                case 0X00280304: return 2039;
                case 0X00280400: return 2040;
                case 0X00280401: return 2041;
                case 0X00280402: return 2042;
                case 0X00280403: return 2043;
                case 0X00280404: return 2044;
                case 0X00280700: return 2045;
                case 0X00280701: return 2046;
                case 0X00280702: return 2047;
                case 0X00280710: return 2048;
                case 0X00280720: return 2049;
                case 0X00280721: return 2050;
                case 0X00280722: return 2051;
                case 0X00280730: return 2052;
                case 0X00280740: return 2053;
                case 0X00280A02: return 2054;
                case 0X00280A04: return 2055;
                case 0X00281040: return 2056;
                case 0X00281041: return 2057;
                case 0X00281050: return 2058;
                case 0X00281051: return 2059;
                case 0X00281052: return 2060;
                case 0X00281053: return 2061;
                case 0X00281054: return 2062;
                case 0X00281055: return 2063;
                case 0X00281056: return 2064;
                case 0X00281080: return 2065;
                case 0X00281090: return 2066;
                case 0X00281100: return 2067;
                case 0X00281101: return 2068;
                case 0X00281102: return 2069;
                case 0X00281103: return 2070;
                case 0X00281104: return 2071;
                case 0X00281111: return 2072;
                case 0X00281112: return 2073;
                case 0X00281113: return 2074;
                case 0X00281199: return 2075;
                case 0X00281200: return 2076;
                case 0X00281201: return 2077;
                case 0X00281202: return 2078;
                case 0X00281203: return 2079;
                case 0X00281204: return 2080;
                case 0X00281211: return 2081;
                case 0X00281212: return 2082;
                case 0X00281213: return 2083;
                case 0X00281214: return 2084;
                case 0X00281221: return 2085;
                case 0X00281222: return 2086;
                case 0X00281223: return 2087;
                case 0X00281224: return 2088;
                case 0X00281230: return 2089;
                case 0X00281231: return 2090;
                case 0X00281232: return 2091;
                case 0X00281300: return 2092;
                case 0X00281350: return 2093;
                case 0X00281351: return 2094;
                case 0X00281352: return 2095;
                case 0X0028135A: return 2096;
                case 0X00281401: return 2097;
                case 0X00281402: return 2098;
                case 0X00281403: return 2099;
                case 0X00281404: return 2100;
                case 0X00281405: return 2101;
                case 0X00281406: return 2102;
                case 0X00281407: return 2103;
                case 0X00281408: return 2104;
                case 0X0028140B: return 2105;
                case 0X0028140C: return 2106;
                case 0X0028140D: return 2107;
                case 0X0028140E: return 2108;
                case 0X0028140F: return 2109;
                case 0X00281410: return 2110;
                case 0X00282000: return 2111;
                case 0X00282002: return 2112;
                case 0X00282110: return 2113;
                case 0X00282112: return 2114;
                case 0X00282114: return 2115;
                case 0X00283000: return 2116;
                case 0X00283002: return 2117;
                case 0X00283003: return 2118;
                case 0X00283004: return 2119;
                case 0X00283006: return 2120;
                case 0X00283010: return 2121;
                case 0X00283110: return 2122;
                case 0X00284000: return 2123;
                case 0X00285000: return 2124;
                case 0X00286010: return 2125;
                case 0X00286020: return 2126;
                case 0X00286022: return 2127;
                case 0X00286023: return 2128;
                case 0X00286030: return 2129;
                case 0X00286040: return 2130;
                case 0X00286100: return 2131;
                case 0X00286101: return 2132;
                case 0X00286102: return 2133;
                case 0X00286110: return 2134;
                case 0X00286112: return 2135;
                case 0X00286114: return 2136;
                case 0X00286120: return 2137;
                case 0X00286190: return 2138;
                case 0X00287000: return 2139;
                case 0X00287001: return 2140;
                case 0X00287002: return 2141;
                case 0X00287003: return 2142;
                case 0X00287004: return 2143;
                case 0X00287005: return 2144;
                case 0X00287006: return 2145;
                case 0X00287007: return 2146;
                case 0X00287008: return 2147;
                case 0X00287009: return 2148;
                case 0X0028700A: return 2149;
                case 0X0028700B: return 2150;
                case 0X0028700C: return 2151;
                case 0X0028700D: return 2152;
                case 0X0028700E: return 2153;
                case 0X0028700F: return 2154;
                case 0X00287010: return 2155;
                case 0X00287011: return 2156;
                case 0X00287012: return 2157;
                case 0X00287013: return 2158;
                case 0X00287014: return 2159;
                case 0X00287015: return 2160;
                case 0X00287016: return 2161;
                case 0X00287017: return 2162;
                case 0X00287018: return 2163;
                case 0X00287019: return 2164;
                case 0X0028701A: return 2165;
                case 0X0028701B: return 2166;
                case 0X0028701C: return 2167;
                case 0X0028701D: return 2168;
                case 0X0028701E: return 2169;
                case 0X0028701F: return 2170;
                case 0X00287020: return 2171;
                case 0X00287021: return 2172;
                case 0X00287022: return 2173;
                case 0X00287023: return 2174;
                case 0X00287024: return 2175;
                case 0X00287025: return 2176;
                case 0X00287026: return 2177;
                case 0X00287027: return 2178;
                case 0X00287028: return 2179;
                case 0X00287029: return 2180;
                case 0X0028702A: return 2181;
                case 0X0028702B: return 2182;
                case 0X0028702C: return 2183;
                case 0X0028702D: return 2184;
                case 0X0028702E: return 2185;
                case 0X00287FE0: return 2186;
                case 0X00289001: return 2187;
                case 0X00289002: return 2188;
                case 0X00289003: return 2189;
                case 0X00289099: return 2190;
                case 0X00289108: return 2191;
                case 0X00289110: return 2192;
                case 0X00289132: return 2193;
                case 0X00289145: return 2194;
                case 0X00289235: return 2195;
                case 0X00289411: return 2196;
                case 0X00289415: return 2197;
                case 0X00289416: return 2198;
                case 0X00289422: return 2199;
                case 0X00289443: return 2200;
                case 0X00289444: return 2201;
                case 0X00289445: return 2202;
                case 0X00289446: return 2203;
                case 0X00289454: return 2204;
                case 0X00289474: return 2205;
                case 0X00289478: return 2206;
                case 0X00289501: return 2207;
                case 0X00289502: return 2208;
                case 0X00289503: return 2209;
                case 0X00289505: return 2210;
                case 0X00289506: return 2211;
                case 0X00289507: return 2212;
                case 0X00289520: return 2213;
                case 0X00289537: return 2214;
                default:         return -1;
            }

        case 0X0032:
            switch (tag) {
                case 0X0032000A: return 2215;
                case 0X0032000C: return 2216;
                case 0X00320012: return 2217;
                case 0X00320032: return 2218;
                case 0X00320033: return 2219;
                case 0X00320034: return 2220;
                case 0X00320035: return 2221;
                case 0X00321000: return 2222;
                case 0X00321001: return 2223;
                case 0X00321010: return 2224;
                case 0X00321011: return 2225;
                case 0X00321020: return 2226;
                case 0X00321021: return 2227;
                case 0X00321030: return 2228;
                case 0X00321031: return 2229;
                case 0X00321032: return 2230;
                case 0X00321033: return 2231;
                case 0X00321034: return 2232;
                case 0X00321040: return 2233;
                case 0X00321041: return 2234;
                case 0X00321050: return 2235;
                case 0X00321051: return 2236;
                case 0X00321055: return 2237;
                case 0X00321060: return 2238;
                case 0X00321064: return 2239;
                case 0X00321066: return 2240;
                case 0X00321067: return 2241;
                case 0X00321070: return 2242;
                case 0X00324000: return 2243;
                default:         return -1;
            }

        case 0X0034:
            switch (tag) {
                case 0X00340001: return 2244;
                case 0X00340002: return 2245;
                case 0X00340003: return 2246;
                case 0X00340004: return 2247;
                case 0X00340005: return 2248;
                case 0X00340007: return 2249;
                case 0X00340008: return 2250;
                case 0X00340009: return 2251;
                case 0X0034000A: return 2252;
                case 0X0034000B: return 2253;
                case 0X0034000C: return 2254;
                case 0X0034000D: return 2255;
                default:         return -1;
            }

        case 0X0038:
            switch (tag) {
                case 0X00380004: return 2256;
                case 0X00380008: return 2257;
                case 0X00380010: return 2258;
                case 0X00380011: return 2259;
                case 0X00380014: return 2260;
                case 0X00380016: return 2261;
                case 0X0038001A: return 2262;
                case 0X0038001B: return 2263;
                case 0X0038001C: return 2264;
                case 0X0038001D: return 2265;
                case 0X0038001E: return 2266;
                case 0X00380020: return 2267;
                case 0X00380021: return 2268;
                case 0X00380030: return 2269;
                case 0X00380032: return 2270;
                case 0X00380040: return 2271;
                case 0X00380044: return 2272;
                case 0X00380050: return 2273;
                case 0X00380060: return 2274;
                case 0X00380061: return 2275;
                case 0X00380062: return 2276;
                case 0X00380064: return 2277;
                case 0X00380100: return 2278;
                case 0X00380101: return 2279;
                case 0X00380102: return 2280;
                case 0X00380300: return 2281;
                case 0X00380400: return 2282;
                case 0X00380500: return 2283;
                case 0X00380502: return 2284;
                case 0X00384000: return 2285;
                default:         return -1;
            }

        case 0X003A:
            switch (tag) {
                case 0X003A0004: return 2286;
                case 0X003A0005: return 2287;
                case 0X003A0010: return 2288;
                case 0X003A001A: return 2289;
                case 0X003A0020: return 2290;
                case 0X003A0200: return 2291;
                case 0X003A0202: return 2292;
                case 0X003A0203: return 2293;
                case 0X003A0205: return 2294;
                case 0X003A0208: return 2295;
                case 0X003A0209: return 2296;
                case 0X003A020A: return 2297;
                case 0X003A020C: return 2298;
                case 0X003A0210: return 2299;
                case 0X003A0211: return 2300;
                case 0X003A0212: return 2301;
                case 0X003A0213: return 2302;
                case 0X003A0214: return 2303;
                case 0X003A0215: return 2304;
                case 0X003A0218: return 2305;
                case 0X003A021A: return 2306;
                case 0X003A0220: return 2307;
                case 0X003A0221: return 2308;
                case 0X003A0222: return 2309;
                case 0X003A0223: return 2310;
                case 0X003A0230: return 2311;
                case 0X003A0231: return 2312;
                case 0X003A0240: return 2313;
                case 0X003A0241: return 2314;
                case 0X003A0242: return 2315;
                case 0X003A0244: return 2316;
                case 0X003A0245: return 2317;
                case 0X003A0246: return 2318;
                case 0X003A0247: return 2319;
                case 0X003A0248: return 2320;
                case 0X003A0300: return 2321;
                case 0X003A0301: return 2322;
                case 0X003A0302: return 2323;
                case 0X003A0310: return 2324;
                case 0X003A0311: return 2325;
                case 0X003A0312: return 2326;
                case 0X003A0313: return 2327;
                case 0X003A0314: return 2328;
                case 0X003A0315: return 2329;
                case 0X003A0316: return 2330;
                default:         return -1;
            }

        case 0X0040:
            switch (tag) {
                case 0X00400001: return 2331;
                case 0X00400002: return 2332;
                case 0X00400003: return 2333;
                case 0X00400004: return 2334;
                case 0X00400005: return 2335;
                case 0X00400006: return 2336;
                case 0X00400007: return 2337;
                case 0X00400008: return 2338;
                case 0X00400009: return 2339;
                case 0X0040000A: return 2340;
                case 0X0040000B: return 2341;
                case 0X00400010: return 2342;
                case 0X00400011: return 2343;
                case 0X00400012: return 2344;
                case 0X00400020: return 2345;
                case 0X00400026: return 2346;
                case 0X00400027: return 2347;
                case 0X00400031: return 2348;
                case 0X00400032: return 2349;
                case 0X00400033: return 2350;
                case 0X00400035: return 2351;
                case 0X00400036: return 2352;
                case 0X00400039: return 2353;
                case 0X0040003A: return 2354;
                case 0X00400100: return 2355;
                case 0X00400220: return 2356;
                case 0X00400241: return 2357;
                case 0X00400242: return 2358;
                case 0X00400243: return 2359;
                case 0X00400244: return 2360;
                case 0X00400245: return 2361;
                case 0X00400250: return 2362;
                case 0X00400251: return 2363;
                case 0X00400252: return 2364;
                case 0X00400253: return 2365;
                case 0X00400254: return 2366;
                case 0X00400255: return 2367;
                case 0X00400260: return 2368;
                case 0X00400261: return 2369;
                case 0X00400270: return 2370;
                case 0X00400275: return 2371;
                case 0X00400280: return 2372;
                case 0X00400281: return 2373;
                case 0X00400293: return 2374;
                case 0X00400294: return 2375;
                case 0X00400295: return 2376;
                case 0X00400296: return 2377;
                case 0X00400300: return 2378;
                case 0X00400301: return 2379;
                case 0X00400302: return 2380;
                case 0X00400303: return 2381;
                case 0X00400306: return 2382;
                case 0X00400307: return 2383;
                case 0X0040030E: return 2384;
                case 0X00400310: return 2385;
                case 0X00400312: return 2386;
                case 0X00400314: return 2387;
                case 0X00400316: return 2388;
                case 0X00400318: return 2389;
                case 0X00400320: return 2390;
                case 0X00400321: return 2391;
                case 0X00400324: return 2392;
                case 0X00400330: return 2393;
                case 0X00400340: return 2394;
                case 0X00400400: return 2395;
                case 0X00400440: return 2396;
                case 0X00400441: return 2397;
                case 0X00400500: return 2398;
                case 0X0040050A: return 2399;
                case 0X00400512: return 2400;
                case 0X00400513: return 2401;
                case 0X00400515: return 2402;
                case 0X00400518: return 2403;
                case 0X0040051A: return 2404;
                case 0X00400520: return 2405;
                case 0X00400550: return 2406;
                case 0X00400551: return 2407;
                case 0X00400552: return 2408;
                case 0X00400553: return 2409;
                case 0X00400554: return 2410;
                case 0X00400555: return 2411;
                case 0X00400556: return 2412;
                case 0X00400560: return 2413;
                case 0X00400562: return 2414;
                case 0X0040059A: return 2415;
                case 0X00400600: return 2416;
                case 0X00400602: return 2417;
                case 0X00400610: return 2418;
                case 0X00400612: return 2419;
                case 0X00400620: return 2420;
                case 0X004006FA: return 2421;
                case 0X00400710: return 2422;
                case 0X0040071A: return 2423;
                case 0X0040072A: return 2424;
                case 0X0040073A: return 2425;
                case 0X0040074A: return 2426;
                case 0X004008D8: return 2427;
                case 0X004008DA: return 2428;
                case 0X004008EA: return 2429;
                case 0X004009F8: return 2430;
                case 0X00401001: return 2431;
                case 0X00401002: return 2432;
                case 0X00401003: return 2433;
                case 0X00401004: return 2434;
                case 0X00401005: return 2435;
                case 0X00401006: return 2436;
                case 0X00401007: return 2437;
                case 0X00401008: return 2438;
                case 0X00401009: return 2439;
                case 0X0040100A: return 2440;
                case 0X00401010: return 2441;
                case 0X00401011: return 2442;
                case 0X00401012: return 2443;
                case 0X00401060: return 2444;
                case 0X00401101: return 2445;
                case 0X00401102: return 2446;
                case 0X00401103: return 2447;
                case 0X00401104: return 2448;
                case 0X00401400: return 2449;
                case 0X00402001: return 2450;
                case 0X00402004: return 2451;
                case 0X00402005: return 2452;
                case 0X00402006: return 2453;
                case 0X00402007: return 2454;
                case 0X00402008: return 2455;
                case 0X00402009: return 2456;
                case 0X00402010: return 2457;
                case 0X00402011: return 2458;
                case 0X00402016: return 2459;
                case 0X00402017: return 2460;
                case 0X00402400: return 2461;
                case 0X00403001: return 2462;
                case 0X00404001: return 2463;
                case 0X00404002: return 2464;
                case 0X00404003: return 2465;
                case 0X00404004: return 2466;
                case 0X00404005: return 2467;
                case 0X00404006: return 2468;
                case 0X00404007: return 2469;
                case 0X00404008: return 2470;
                case 0X00404009: return 2471;
                case 0X00404010: return 2472;
                case 0X00404011: return 2473;
                case 0X00404015: return 2474;
                case 0X00404016: return 2475;
                case 0X00404018: return 2476;
                case 0X00404019: return 2477;
                case 0X00404020: return 2478;
                case 0X00404021: return 2479;
                case 0X00404022: return 2480;
                case 0X00404023: return 2481;
                case 0X00404025: return 2482;
                case 0X00404026: return 2483;
                case 0X00404027: return 2484;
                case 0X00404028: return 2485;
                case 0X00404029: return 2486;
                case 0X00404030: return 2487;
                case 0X00404031: return 2488;
                case 0X00404032: return 2489;
                case 0X00404033: return 2490;
                case 0X00404034: return 2491;
                case 0X00404035: return 2492;
                case 0X00404036: return 2493;
                case 0X00404037: return 2494;
                case 0X00404040: return 2495;
                case 0X00404041: return 2496;
                case 0X00404050: return 2497;
                case 0X00404051: return 2498;
                case 0X00404052: return 2499;
                case 0X00404070: return 2500;
                case 0X00404071: return 2501;
                case 0X00404072: return 2502;
                case 0X00404073: return 2503;
                case 0X00404074: return 2504;
                case 0X00408302: return 2505;
                case 0X00408303: return 2506;
                case 0X00409092: return 2507;
                case 0X00409094: return 2508;
                case 0X00409096: return 2509;
                case 0X00409098: return 2510;
                case 0X00409210: return 2511;
                case 0X00409211: return 2512;
                case 0X00409212: return 2513;
                case 0X00409213: return 2514;
                case 0X00409214: return 2515;
                case 0X00409216: return 2516;
                case 0X00409220: return 2517;
                case 0X00409224: return 2518;
                case 0X00409225: return 2519;
                case 0X0040A007: return 2520;
                case 0X0040A010: return 2521;
                case 0X0040A020: return 2522;
                case 0X0040A021: return 2523;
                case 0X0040A022: return 2524;
                case 0X0040A023: return 2525;
                case 0X0040A024: return 2526;
                case 0X0040A026: return 2527;
                case 0X0040A027: return 2528;
                case 0X0040A028: return 2529;
                case 0X0040A030: return 2530;
                case 0X0040A032: return 2531;
                case 0X0040A033: return 2532;
                case 0X0040A040: return 2533;
                case 0X0040A043: return 2534;
                case 0X0040A047: return 2535;
                case 0X0040A050: return 2536;
                case 0X0040A057: return 2537;
                case 0X0040A060: return 2538;
                case 0X0040A066: return 2539;
                case 0X0040A067: return 2540;
                case 0X0040A068: return 2541;
                case 0X0040A070: return 2542;
                case 0X0040A073: return 2543;
                case 0X0040A074: return 2544;
                case 0X0040A075: return 2545;
                case 0X0040A076: return 2546;
                case 0X0040A078: return 2547;
                case 0X0040A07A: return 2548;
                case 0X0040A07C: return 2549;
                case 0X0040A080: return 2550;
                case 0X0040A082: return 2551;
                case 0X0040A084: return 2552;
                case 0X0040A085: return 2553;
                case 0X0040A088: return 2554;
                case 0X0040A089: return 2555;
                case 0X0040A090: return 2556;
                case 0X0040A0B0: return 2557;
                case 0X0040A110: return 2558;
                case 0X0040A112: return 2559;
                case 0X0040A120: return 2560;
                case 0X0040A121: return 2561;
                case 0X0040A122: return 2562;
                case 0X0040A123: return 2563;
                case 0X0040A124: return 2564;
                case 0X0040A125: return 2565;
                case 0X0040A130: return 2566;
                case 0X0040A132: return 2567;
                case 0X0040A136: return 2568;
                case 0X0040A138: return 2569;
                case 0X0040A13A: return 2570;
                case 0X0040A160: return 2571;
                case 0X0040A161: return 2572;
                case 0X0040A162: return 2573;
                case 0X0040A163: return 2574;
                case 0X0040A167: return 2575;
                case 0X0040A168: return 2576;
                case 0X0040A16A: return 2577;
                case 0X0040A170: return 2578;
                case 0X0040A171: return 2579;
                case 0X0040A172: return 2580;
                case 0X0040A173: return 2581;
                case 0X0040A174: return 2582;
                case 0X0040A180: return 2583;
                case 0X0040A192: return 2584;
                case 0X0040A193: return 2585;
                case 0X0040A194: return 2586;
                case 0X0040A195: return 2587;
                case 0X0040A224: return 2588;
                case 0X0040A290: return 2589;
                case 0X0040A296: return 2590;
                case 0X0040A297: return 2591;
                case 0X0040A29A: return 2592;
                case 0X0040A300: return 2593;
                case 0X0040A301: return 2594;
                case 0X0040A307: return 2595;
                case 0X0040A30A: return 2596;
                case 0X0040A313: return 2597;
                case 0X0040A33A: return 2598;
                case 0X0040A340: return 2599;
                case 0X0040A352: return 2600;
                case 0X0040A353: return 2601;
                case 0X0040A354: return 2602;
                case 0X0040A358: return 2603;
                case 0X0040A360: return 2604;
                case 0X0040A370: return 2605;
                case 0X0040A372: return 2606;
                case 0X0040A375: return 2607;
                case 0X0040A380: return 2608;
                case 0X0040A385: return 2609;
                case 0X0040A390: return 2610;
                case 0X0040A402: return 2611;
                case 0X0040A403: return 2612;
                case 0X0040A404: return 2613;
                case 0X0040A491: return 2614;
                case 0X0040A492: return 2615;
                case 0X0040A493: return 2616;
                case 0X0040A494: return 2617;
                case 0X0040A496: return 2618;
                case 0X0040A504: return 2619;
                case 0X0040A525: return 2620;
                case 0X0040A600: return 2621;
                case 0X0040A601: return 2622;
                case 0X0040A603: return 2623;
                case 0X0040A730: return 2624;
                case 0X0040A731: return 2625;
                case 0X0040A732: return 2626;
                case 0X0040A744: return 2627;
                case 0X0040A801: return 2628;
                case 0X0040A802: return 2629;
                case 0X0040A803: return 2630;
                case 0X0040A804: return 2631;
                case 0X0040A805: return 2632;
                case 0X0040A806: return 2633;
                case 0X0040A807: return 2634;
                case 0X0040A808: return 2635;
                case 0X0040A992: return 2636;
                case 0X0040B020: return 2637;
                case 0X0040DB00: return 2638;
                case 0X0040DB06: return 2639;
                case 0X0040DB07: return 2640;
                case 0X0040DB0B: return 2641;
                case 0X0040DB0C: return 2642;
                case 0X0040DB0D: return 2643;
                case 0X0040DB73: return 2644;
                case 0X0040E001: return 2645;
                case 0X0040E004: return 2646;
                case 0X0040E006: return 2647;
                case 0X0040E008: return 2648;
                case 0X0040E010: return 2649;
                case 0X0040E011: return 2650;
                case 0X0040E020: return 2651;
                case 0X0040E021: return 2652;
                case 0X0040E022: return 2653;
                case 0X0040E023: return 2654;
                case 0X0040E024: return 2655;
                case 0X0040E025: return 2656;
                case 0X0040E030: return 2657;
                case 0X0040E031: return 2658;
                default:         return -1;
            }

        case 0X0042:
            switch (tag) {
                case 0X00420010: return 2659;
                case 0X00420011: return 2660;
                case 0X00420012: return 2661;
                case 0X00420013: return 2662;
                case 0X00420014: return 2663;
                case 0X00420015: return 2664;
                default:         return -1;
            }

        case 0X0044:
            switch (tag) {
                case 0X00440001: return 2665;
                case 0X00440002: return 2666;
                case 0X00440003: return 2667;
                case 0X00440004: return 2668;
                case 0X00440007: return 2669;
                case 0X00440008: return 2670;
                case 0X00440009: return 2671;
                case 0X0044000A: return 2672;
                case 0X0044000B: return 2673;
                case 0X00440010: return 2674;
                case 0X00440011: return 2675;
                case 0X00440012: return 2676;
                case 0X00440013: return 2677;
                case 0X00440019: return 2678;
                case 0X00440100: return 2679;
                case 0X00440101: return 2680;
                case 0X00440102: return 2681;
                case 0X00440103: return 2682;
                case 0X00440104: return 2683;
                case 0X00440105: return 2684;
                case 0X00440106: return 2685;
                case 0X00440107: return 2686;
                case 0X00440108: return 2687;
                case 0X00440109: return 2688;
                case 0X0044010A: return 2689;
                default:         return -1;
            }

        case 0X0046:
            switch (tag) {
                case 0X00460012: return 2690;
                case 0X00460014: return 2691;
                case 0X00460015: return 2692;
                case 0X00460016: return 2693;
                case 0X00460018: return 2694;
                case 0X00460028: return 2695;
                case 0X00460030: return 2696;
                case 0X00460032: return 2697;
                case 0X00460034: return 2698;
                case 0X00460036: return 2699;
                case 0X00460038: return 2700;
                case 0X00460040: return 2701;
                case 0X00460042: return 2702;
                case 0X00460044: return 2703;
                case 0X00460046: return 2704;
                case 0X00460047: return 2705;
                case 0X00460050: return 2706;
                case 0X00460052: return 2707;
                case 0X00460060: return 2708;
                case 0X00460062: return 2709;
                case 0X00460063: return 2710;
                case 0X00460064: return 2711;
                case 0X00460070: return 2712;
                case 0X00460071: return 2713;
                case 0X00460074: return 2714;
                case 0X00460075: return 2715;
                case 0X00460076: return 2716;
                case 0X00460077: return 2717;
                case 0X00460080: return 2718;
                case 0X00460092: return 2719;
                case 0X00460094: return 2720;
                case 0X00460095: return 2721;
                case 0X00460097: return 2722;
                case 0X00460098: return 2723;
                case 0X00460100: return 2724;
                case 0X00460101: return 2725;
                case 0X00460102: return 2726;
                case 0X00460104: return 2727;
                case 0X00460106: return 2728;
                case 0X00460110: return 2729;
                case 0X00460111: return 2730;
                case 0X00460112: return 2731;
                case 0X00460113: return 2732;
                case 0X00460114: return 2733;
                case 0X00460115: return 2734;
                case 0X00460116: return 2735;
                case 0X00460117: return 2736;
                case 0X00460118: return 2737;
                case 0X00460121: return 2738;
                case 0X00460122: return 2739;
                case 0X00460123: return 2740;
                case 0X00460124: return 2741;
                case 0X00460125: return 2742;
                case 0X00460135: return 2743;
                case 0X00460137: return 2744;
                case 0X00460139: return 2745;
                case 0X00460145: return 2746;
                case 0X00460146: return 2747;
                case 0X00460147: return 2748;
                case 0X00460201: return 2749;
                case 0X00460202: return 2750;
                case 0X00460203: return 2751;
                case 0X00460204: return 2752;
                case 0X00460205: return 2753;
                case 0X00460207: return 2754;
                case 0X00460208: return 2755;
                case 0X00460210: return 2756;
                case 0X00460211: return 2757;
                case 0X00460212: return 2758;
                case 0X00460213: return 2759;
                case 0X00460215: return 2760;
                case 0X00460218: return 2761;
                case 0X00460220: return 2762;
                case 0X00460224: return 2763;
                case 0X00460227: return 2764;
                case 0X00460230: return 2765;
                case 0X00460232: return 2766;
                case 0X00460234: return 2767;
                case 0X00460236: return 2768;
                case 0X00460238: return 2769;
                case 0X00460242: return 2770;
                case 0X00460244: return 2771;
                case 0X00460247: return 2772;
                case 0X00460248: return 2773;
                case 0X00460249: return 2774;
                case 0X00460250: return 2775;
                case 0X00460251: return 2776;
                case 0X00460252: return 2777;
                case 0X00460253: return 2778;
                default:         return -1;
            }

        case 0X0048:
            switch (tag) {
                case 0X00480001: return 2779;
                case 0X00480002: return 2780;
                case 0X00480003: return 2781;
                case 0X00480006: return 2782;
                case 0X00480007: return 2783;
                case 0X00480008: return 2784;
                case 0X00480010: return 2785;
                case 0X00480011: return 2786;
                case 0X00480012: return 2787;
                case 0X00480013: return 2788;
                case 0X00480014: return 2789;
                case 0X00480015: return 2790;
                case 0X00480100: return 2791;
                case 0X00480102: return 2792;
                case 0X00480105: return 2793;
                case 0X00480106: return 2794;
                case 0X00480107: return 2795;
                case 0X00480108: return 2796;
                case 0X00480110: return 2797;
                case 0X00480111: return 2798;
                case 0X00480112: return 2799;
                case 0X00480113: return 2800;
                case 0X00480120: return 2801;
                case 0X00480200: return 2802;
                case 0X00480201: return 2803;
                case 0X00480202: return 2804;
                case 0X00480207: return 2805;
                case 0X0048021A: return 2806;
                case 0X0048021E: return 2807;
                case 0X0048021F: return 2808;
                case 0X00480301: return 2809;
                case 0X00480302: return 2810;
                case 0X00480303: return 2811;
                default:         return -1;
            }

        case 0X0050:
            switch (tag) {
                case 0X00500004: return 2812;
                case 0X00500010: return 2813;
                case 0X00500012: return 2814;
                case 0X00500013: return 2815;
                case 0X00500014: return 2816;
                case 0X00500015: return 2817;
                case 0X00500016: return 2818;
                case 0X00500017: return 2819;
                case 0X00500018: return 2820;
                case 0X00500019: return 2821;
                case 0X0050001A: return 2822;
                case 0X0050001B: return 2823;
                case 0X0050001C: return 2824;
                case 0X0050001D: return 2825;
                case 0X0050001E: return 2826;
                case 0X00500020: return 2827;
                case 0X00500021: return 2828;
                default:         return -1;
            }

        case 0X0052:
            switch (tag) {
                case 0X00520001: return 2829;
                case 0X00520002: return 2830;
                case 0X00520003: return 2831;
                case 0X00520004: return 2832;
                case 0X00520006: return 2833;
                case 0X00520007: return 2834;
                case 0X00520008: return 2835;
                case 0X00520009: return 2836;
                case 0X00520011: return 2837;
                case 0X00520012: return 2838;
                case 0X00520013: return 2839;
                case 0X00520014: return 2840;
                case 0X00520016: return 2841;
                case 0X00520025: return 2842;
                case 0X00520026: return 2843;
                case 0X00520027: return 2844;
                case 0X00520028: return 2845;
                case 0X00520029: return 2846;
                case 0X00520030: return 2847;
                case 0X00520031: return 2848;
                case 0X00520033: return 2849;
                case 0X00520034: return 2850;
                case 0X00520036: return 2851;
                case 0X00520038: return 2852;
                case 0X00520039: return 2853;
                case 0X0052003A: return 2854;
                default:         return -1;
            }

        case 0X0054:
            switch (tag) {
                case 0X00540010: return 2855;
                case 0X00540011: return 2856;
                case 0X00540012: return 2857;
                case 0X00540013: return 2858;
                case 0X00540014: return 2859;
                case 0X00540015: return 2860;
                case 0X00540016: return 2861;
                case 0X00540017: return 2862;
                case 0X00540018: return 2863;
                case 0X00540020: return 2864;
                case 0X00540021: return 2865;
                case 0X00540022: return 2866;
                case 0X00540030: return 2867;
                case 0X00540031: return 2868;
                case 0X00540032: return 2869;
                case 0X00540033: return 2870;
                case 0X00540036: return 2871;
                case 0X00540038: return 2872;
                case 0X00540039: return 2873;
                case 0X00540050: return 2874;
                case 0X00540051: return 2875;
                case 0X00540052: return 2876;
                case 0X00540053: return 2877;
                case 0X00540060: return 2878;
                case 0X00540061: return 2879;
                case 0X00540062: return 2880;
                case 0X00540063: return 2881;
                case 0X00540070: return 2882;
                case 0X00540071: return 2883;
                case 0X00540072: return 2884;
                case 0X00540073: return 2885;
                case 0X00540080: return 2886;
                case 0X00540081: return 2887;
                case 0X00540090: return 2888;
                case 0X00540100: return 2889;
                case 0X00540101: return 2890;
                case 0X00540200: return 2891;
                case 0X00540202: return 2892;
                case 0X00540210: return 2893;
                case 0X00540211: return 2894;
                case 0X00540220: return 2895;
                case 0X00540222: return 2896;
                case 0X00540300: return 2897;
                case 0X00540302: return 2898;
                case 0X00540304: return 2899;
                case 0X00540306: return 2900;
                case 0X00540308: return 2901;
                case 0X00540400: return 2902;
                case 0X00540410: return 2903;
                case 0X00540412: return 2904;
                case 0X00540414: return 2905;
                case 0X00540500: return 2906;
                case 0X00540501: return 2907;
                case 0X00541000: return 2908;
                case 0X00541001: return 2909;
                case 0X00541002: return 2910;
                case 0X00541004: return 2911;
                case 0X00541006: return 2912;
                case 0X00541100: return 2913;
                case 0X00541101: return 2914;
                case 0X00541102: return 2915;
                case 0X00541103: return 2916;
                case 0X00541104: return 2917;
                case 0X00541105: return 2918;
                case 0X00541200: return 2919;
                case 0X00541201: return 2920;
                case 0X00541202: return 2921;
                case 0X00541203: return 2922;
                case 0X00541210: return 2923;
                case 0X00541220: return 2924;
                case 0X00541300: return 2925;
                case 0X00541310: return 2926;
                case 0X00541311: return 2927;
                case 0X00541320: return 2928;
                case 0X00541321: return 2929;
                case 0X00541322: return 2930;
                case 0X00541323: return 2931;
                case 0X00541324: return 2932;
                case 0X00541330: return 2933;
                case 0X00541400: return 2934;
                case 0X00541401: return 2935;
                default:         return -1;
            }

        case 0X0060:
            switch (tag) {
                case 0X00603000: return 2936;
                case 0X00603002: return 2937;
                case 0X00603004: return 2938;
                case 0X00603006: return 2939;
                case 0X00603008: return 2940;
                case 0X00603010: return 2941;
                case 0X00603020: return 2942;
                default:         return -1;
            }

        case 0X0062:
            switch (tag) {
                case 0X00620001: return 2943;
                case 0X00620002: return 2944;
                case 0X00620003: return 2945;
                case 0X00620004: return 2946;
                case 0X00620005: return 2947;
                case 0X00620006: return 2948;
                case 0X00620007: return 2949;
                case 0X00620008: return 2950;
                case 0X00620009: return 2951;
                case 0X0062000A: return 2952;
                case 0X0062000B: return 2953;
                case 0X0062000C: return 2954;
                case 0X0062000D: return 2955;
                case 0X0062000E: return 2956;
                case 0X0062000F: return 2957;
                case 0X00620010: return 2958;
                case 0X00620011: return 2959;
                case 0X00620012: return 2960;
                case 0X00620013: return 2961;
                case 0X00620020: return 2962;
                case 0X00620021: return 2963;
                default:         return -1;
            }

        case 0X0064:
            switch (tag) {
                case 0X00640002: return 2964;
                case 0X00640003: return 2965;
                case 0X00640005: return 2966;
                case 0X00640007: return 2967;
                case 0X00640008: return 2968;
                case 0X00640009: return 2969;
                case 0X0064000F: return 2970;
                case 0X00640010: return 2971;
                default:         return -1;
            }

        case 0X0066:
            switch (tag) {
                case 0X00660001: return 2972;
                case 0X00660002: return 2973;
                case 0X00660003: return 2974;
                case 0X00660004: return 2975;
                case 0X00660009: return 2976;
                case 0X0066000A: return 2977;
                case 0X0066000B: return 2978;
                case 0X0066000C: return 2979;
                case 0X0066000D: return 2980;
                case 0X0066000E: return 2981;
                case 0X00660010: return 2982;
                case 0X00660011: return 2983;
                case 0X00660012: return 2984;
                case 0X00660013: return 2985;
                case 0X00660015: return 2986;
                case 0X00660016: return 2987;
                case 0X00660017: return 2988;
                case 0X00660018: return 2989;
                case 0X00660019: return 2990;
                case 0X0066001A: return 2991;
                case 0X0066001B: return 2992;
                case 0X0066001C: return 2993;
                case 0X0066001E: return 2994;
                case 0X0066001F: return 2995;
                case 0X00660020: return 2996;
                case 0X00660021: return 2997;
                case 0X00660022: return 2998;
                case 0X00660023: return 2999;
                case 0X00660024: return 3000;
                case 0X00660025: return 3001;
                case 0X00660026: return 3002;
                case 0X00660027: return 3003;
                case 0X00660028: return 3004;
                case 0X00660029: return 3005;
                case 0X0066002A: return 3006;
                case 0X0066002B: return 3007;
                case 0X0066002C: return 3008;
                case 0X0066002D: return 3009;
                case 0X0066002E: return 3010;
                case 0X0066002F: return 3011;
                case 0X00660030: return 3012;
                case 0X00660031: return 3013;
                case 0X00660032: return 3014;
                case 0X00660034: return 3015;
                case 0X00660035: return 3016;
                case 0X00660036: return 3017;
                case 0X00660037: return 3018;
                case 0X00660038: return 3019;
                case 0X00660040: return 3020;
                case 0X00660041: return 3021;
                case 0X00660042: return 3022;
                case 0X00660043: return 3023;
                case 0X00660101: return 3024;
                case 0X00660102: return 3025;
                case 0X00660103: return 3026;
                case 0X00660104: return 3027;
                case 0X00660105: return 3028;
                case 0X00660106: return 3029;
                case 0X00660107: return 3030;
                case 0X00660108: return 3031;
                case 0X00660121: return 3032;
                case 0X00660124: return 3033;
                case 0X00660125: return 3034;
                case 0X00660129: return 3035;
                case 0X00660130: return 3036;
                case 0X00660132: return 3037;
                case 0X00660133: return 3038;
                case 0X00660134: return 3039;
                default:         return -1;
            }

        case 0X0068:
            switch (tag) {
                case 0X00686210: return 3040;
                case 0X00686221: return 3041;
                case 0X00686222: return 3042;
                case 0X00686223: return 3043;
                case 0X00686224: return 3044;
                case 0X00686225: return 3045;
                case 0X00686226: return 3046;
                case 0X00686230: return 3047;
                case 0X00686260: return 3048;
                case 0X00686265: return 3049;
                case 0X00686270: return 3050;
                case 0X00686280: return 3051;
                case 0X006862A0: return 3052;
                case 0X006862A5: return 3053;
                case 0X006862C0: return 3054;
                case 0X006862D0: return 3055;
                case 0X006862D5: return 3056;
                case 0X006862E0: return 3057;
                case 0X006862F0: return 3058;
                case 0X006862F2: return 3059;
                case 0X00686300: return 3060;
                case 0X00686310: return 3061;
                case 0X00686320: return 3062;
                case 0X00686330: return 3063;
                case 0X00686340: return 3064;
                case 0X00686345: return 3065;
                case 0X00686346: return 3066;
                case 0X00686347: return 3067;
                case 0X00686350: return 3068;
                case 0X00686360: return 3069;
                case 0X00686380: return 3070;
                case 0X00686390: return 3071;
                case 0X006863A0: return 3072;
                case 0X006863A4: return 3073;
                case 0X006863A8: return 3074;
                case 0X006863AC: return 3075;
                case 0X006863B0: return 3076;
                case 0X006863C0: return 3077;
                case 0X006863D0: return 3078;
                case 0X006863E0: return 3079;
                case 0X006863F0: return 3080;
                case 0X00686400: return 3081;
                case 0X00686410: return 3082;
                case 0X00686420: return 3083;
                case 0X00686430: return 3084;
                case 0X00686440: return 3085;
                case 0X00686450: return 3086;
                case 0X00686460: return 3087;
                case 0X00686470: return 3088;
                case 0X00686490: return 3089;
                case 0X006864A0: return 3090;
                case 0X006864C0: return 3091;
                case 0X006864D0: return 3092;
                case 0X006864F0: return 3093;
                case 0X00686500: return 3094;
                case 0X00686510: return 3095;
                case 0X00686520: return 3096;
                case 0X00686530: return 3097;
                case 0X00686540: return 3098;
                case 0X00686545: return 3099;
                case 0X00686550: return 3100;
                case 0X00686560: return 3101;
                case 0X00686590: return 3102;
                case 0X006865A0: return 3103;
                case 0X006865B0: return 3104;
                case 0X006865D0: return 3105;
                case 0X006865E0: return 3106;
                case 0X006865F0: return 3107;
                case 0X00686610: return 3108;
                case 0X00686620: return 3109;
                case 0X00687001: return 3110;
                case 0X00687002: return 3111;
                case 0X00687003: return 3112;
                case 0X00687004: return 3113;
                case 0X00687005: return 3114;
                default:         return -1;
            }

        case 0X006A:
            switch (tag) {
                case 0X006A0001: return 3115;
                case 0X006A0002: return 3116;
                case 0X006A0003: return 3117;
                case 0X006A0005: return 3118;
                case 0X006A0006: return 3119;
                case 0X006A0007: return 3120;
                case 0X006A0008: return 3121;
                case 0X006A0009: return 3122;
                case 0X006A000A: return 3123;
                case 0X006A000B: return 3124;
                case 0X006A000C: return 3125;
                case 0X006A000D: return 3126;
                case 0X006A000E: return 3127;
                case 0X006A000F: return 3128;
                case 0X006A0010: return 3129;
                case 0X006A0011: return 3130;
                default:         return -1;
            }

        case 0X0070:
            switch (tag) {
                case 0X00700001: return 3131;
                case 0X00700002: return 3132;
                case 0X00700003: return 3133;
                case 0X00700004: return 3134;
                case 0X00700005: return 3135;
                case 0X00700006: return 3136;
                case 0X00700008: return 3137;
                case 0X00700009: return 3138;
                case 0X00700010: return 3139;
                case 0X00700011: return 3140;
                case 0X00700012: return 3141;
                case 0X00700014: return 3142;
                case 0X00700015: return 3143;
                case 0X00700020: return 3144;
                case 0X00700021: return 3145;
                case 0X00700022: return 3146;
                case 0X00700023: return 3147;
                case 0X00700024: return 3148;
                case 0X00700040: return 3149;
                case 0X00700041: return 3150;
                case 0X00700042: return 3151;
                case 0X00700050: return 3152;
                case 0X00700051: return 3153;
                case 0X00700052: return 3154;
                case 0X00700053: return 3155;
                case 0X0070005A: return 3156;
                case 0X00700060: return 3157;
                case 0X00700062: return 3158;
                case 0X00700066: return 3159;
                case 0X00700067: return 3160;
                case 0X00700068: return 3161;
                case 0X00700080: return 3162;
                case 0X00700081: return 3163;
                case 0X00700082: return 3164;
                case 0X00700083: return 3165;
                case 0X00700084: return 3166;
                case 0X00700086: return 3167;
                case 0X00700087: return 3168;
                case 0X00700100: return 3169;
                case 0X00700101: return 3170;
                case 0X00700102: return 3171;
                case 0X00700103: return 3172;
                case 0X00700207: return 3173;
                case 0X00700208: return 3174;
                case 0X00700209: return 3175;
                case 0X00700226: return 3176;
                case 0X00700227: return 3177;
                case 0X00700228: return 3178;
                case 0X00700229: return 3179;
                case 0X00700230: return 3180;
                case 0X00700231: return 3181;
                case 0X00700232: return 3182;
                case 0X00700233: return 3183;
                case 0X00700234: return 3184;
                case 0X00700241: return 3185;
                case 0X00700242: return 3186;
                case 0X00700243: return 3187;
                case 0X00700244: return 3188;
                case 0X00700245: return 3189;
                case 0X00700246: return 3190;
                case 0X00700247: return 3191;
                case 0X00700248: return 3192;
                case 0X00700249: return 3193;
                case 0X00700250: return 3194;
                case 0X00700251: return 3195;
                case 0X00700252: return 3196;
                case 0X00700253: return 3197;
                case 0X00700254: return 3198;
                case 0X00700255: return 3199;
                case 0X00700256: return 3200;
                case 0X00700257: return 3201;
                case 0X00700258: return 3202;
                case 0X00700261: return 3203;
                case 0X00700262: return 3204;
                case 0X00700273: return 3205;
                case 0X00700274: return 3206;
                case 0X00700278: return 3207;
                case 0X00700279: return 3208;
                case 0X00700282: return 3209;
                case 0X00700284: return 3210;
                case 0X00700285: return 3211;
                case 0X00700287: return 3212;
                case 0X00700288: return 3213;
                case 0X00700289: return 3214;
                case 0X00700294: return 3215;
                case 0X00700295: return 3216;
                case 0X00700306: return 3217;
                case 0X00700308: return 3218;
                case 0X00700309: return 3219;
                case 0X0070030A: return 3220;
                case 0X0070030B: return 3221;
                case 0X0070030C: return 3222;
                case 0X0070030D: return 3223;
                case 0X0070030F: return 3224;
                case 0X00700310: return 3225;
                case 0X00700311: return 3226;
                case 0X00700312: return 3227;
                case 0X00700314: return 3228;
                case 0X00700318: return 3229;
                case 0X0070031A: return 3230;
                case 0X0070031B: return 3231;
                case 0X0070031C: return 3232;
                case 0X0070031E: return 3233;
                case 0X0070031F: return 3234;
                case 0X00700401: return 3235;
                case 0X00700402: return 3236;
                case 0X00700403: return 3237;
                case 0X00700404: return 3238;
                case 0X00700405: return 3239;
                case 0X00701101: return 3240;
                case 0X00701102: return 3241;
                case 0X00701103: return 3242;
                case 0X00701104: return 3243;
                case 0X00701201: return 3244;
                case 0X00701202: return 3245;
                case 0X00701203: return 3246;
                case 0X00701204: return 3247;
                case 0X00701205: return 3248;
                case 0X00701206: return 3249;
                case 0X00701207: return 3250;
                case 0X00701208: return 3251;
                case 0X00701209: return 3252;
                case 0X0070120A: return 3253;
                case 0X0070120B: return 3254;
                case 0X0070120C: return 3255;
                case 0X0070120D: return 3256;
                case 0X00701301: return 3257;
                case 0X00701302: return 3258;
                case 0X00701303: return 3259;
                case 0X00701304: return 3260;
                case 0X00701305: return 3261;
                case 0X00701306: return 3262;
                case 0X00701309: return 3263;
                case 0X00701501: return 3264;
                case 0X00701502: return 3265;
                case 0X00701503: return 3266;
                case 0X00701505: return 3267;
                case 0X00701507: return 3268;
                case 0X00701508: return 3269;
                case 0X0070150C: return 3270;
                case 0X0070150D: return 3271;
                case 0X00701511: return 3272;
                case 0X00701512: return 3273;
                case 0X00701602: return 3274;
                case 0X00701603: return 3275;
                case 0X00701604: return 3276;
                case 0X00701605: return 3277;
                case 0X00701606: return 3278;
                case 0X00701607: return 3279;
                case 0X00701701: return 3280;
                case 0X00701702: return 3281;
                case 0X00701703: return 3282;
                case 0X00701704: return 3283;
                case 0X00701705: return 3284;
                case 0X00701706: return 3285;
                case 0X00701801: return 3286;
                case 0X00701802: return 3287;
                case 0X00701803: return 3288;
                case 0X00701804: return 3289;
                case 0X00701805: return 3290;
                case 0X00701806: return 3291;
                case 0X00701807: return 3292;
                case 0X00701808: return 3293;
                case 0X00701901: return 3294;
                case 0X00701903: return 3295;
                case 0X00701904: return 3296;
                case 0X00701905: return 3297;
                case 0X00701907: return 3298;
                case 0X00701A01: return 3299;
                case 0X00701A03: return 3300;
                case 0X00701A04: return 3301;
                case 0X00701A05: return 3302;
                case 0X00701A06: return 3303;
                case 0X00701A07: return 3304;
                case 0X00701A08: return 3305;
                case 0X00701A09: return 3306;
                case 0X00701B01: return 3307;
                case 0X00701B02: return 3308;
                case 0X00701B03: return 3309;
                case 0X00701B04: return 3310;
                case 0X00701B06: return 3311;
                case 0X00701B07: return 3312;
                case 0X00701B08: return 3313;
                case 0X00701B11: return 3314;
                case 0X00701B12: return 3315;
                case 0X00701B13: return 3316;
                case 0X00701B14: return 3317;
                default:         return -1;
            }

        case 0X0072:
            switch (tag) {
                case 0X00720002: return 3318;
                case 0X00720004: return 3319;
                case 0X00720006: return 3320;
                case 0X00720008: return 3321;
                case 0X0072000A: return 3322;
                case 0X0072000C: return 3323;
                case 0X0072000E: return 3324;
                case 0X00720010: return 3325;
                case 0X00720012: return 3326;
                case 0X00720014: return 3327;
                case 0X00720020: return 3328;
                case 0X00720022: return 3329;
                case 0X00720024: return 3330;
                case 0X00720026: return 3331;
                case 0X00720028: return 3332;
                case 0X00720030: return 3333;
                case 0X00720032: return 3334;
                case 0X00720034: return 3335;
                case 0X00720038: return 3336;
                case 0X0072003A: return 3337;
                case 0X0072003C: return 3338;
                case 0X0072003E: return 3339;
                case 0X00720040: return 3340;
                case 0X00720050: return 3341;
                case 0X00720052: return 3342;
                case 0X00720054: return 3343;
                case 0X00720056: return 3344;
                case 0X0072005E: return 3345;
                case 0X0072005F: return 3346;
                case 0X00720060: return 3347;
                case 0X00720061: return 3348;
                case 0X00720062: return 3349;
                case 0X00720063: return 3350;
                case 0X00720064: return 3351;
                case 0X00720065: return 3352;
                case 0X00720066: return 3353;
                case 0X00720067: return 3354;
                case 0X00720068: return 3355;
                case 0X00720069: return 3356;
                case 0X0072006A: return 3357;
                case 0X0072006B: return 3358;
                case 0X0072006C: return 3359;
                case 0X0072006D: return 3360;
                case 0X0072006E: return 3361;
                case 0X0072006F: return 3362;
                case 0X00720070: return 3363;
                case 0X00720071: return 3364;
                case 0X00720072: return 3365;
                case 0X00720073: return 3366;
                case 0X00720074: return 3367;
                case 0X00720075: return 3368;
                case 0X00720076: return 3369;
                case 0X00720078: return 3370;
                case 0X0072007A: return 3371;
                case 0X0072007C: return 3372;
                case 0X0072007E: return 3373;
                case 0X0072007F: return 3374;
                case 0X00720080: return 3375;
                case 0X00720081: return 3376;
                case 0X00720082: return 3377;
                case 0X00720083: return 3378;
                case 0X00720100: return 3379;
                case 0X00720102: return 3380;
                case 0X00720104: return 3381;
                case 0X00720106: return 3382;
                case 0X00720108: return 3383;
                case 0X0072010A: return 3384;
                case 0X0072010C: return 3385;
                case 0X0072010E: return 3386;
                case 0X00720200: return 3387;
                case 0X00720202: return 3388;
                case 0X00720203: return 3389;
                case 0X00720204: return 3390;
                case 0X00720206: return 3391;
                case 0X00720208: return 3392;
                case 0X00720210: return 3393;
                case 0X00720212: return 3394;
                case 0X00720214: return 3395;
                case 0X00720216: return 3396;
                case 0X00720218: return 3397;
                case 0X00720300: return 3398;
                case 0X00720302: return 3399;
                case 0X00720304: return 3400;
                case 0X00720306: return 3401;
                case 0X00720308: return 3402;
                case 0X00720310: return 3403;
                case 0X00720312: return 3404;
                case 0X00720314: return 3405;
                case 0X00720316: return 3406;
                case 0X00720318: return 3407;
                case 0X00720320: return 3408;
                case 0X00720330: return 3409;
                case 0X00720400: return 3410;
                case 0X00720402: return 3411;
                case 0X00720404: return 3412;
                case 0X00720406: return 3413;
                case 0X00720420: return 3414;
                case 0X00720421: return 3415;
                case 0X00720422: return 3416;
                case 0X00720424: return 3417;
                case 0X00720427: return 3418;
                case 0X00720430: return 3419;
                case 0X00720432: return 3420;
                case 0X00720434: return 3421;
                case 0X00720500: return 3422;
                case 0X00720510: return 3423;
                case 0X00720512: return 3424;
                case 0X00720514: return 3425;
                case 0X00720516: return 3426;
                case 0X00720520: return 3427;
                case 0X00720600: return 3428;
                case 0X00720602: return 3429;
                case 0X00720604: return 3430;
                case 0X00720700: return 3431;
                case 0X00720702: return 3432;
                case 0X00720704: return 3433;
                case 0X00720705: return 3434;
                case 0X00720706: return 3435;
                case 0X00720710: return 3436;
                case 0X00720712: return 3437;
                case 0X00720714: return 3438;
                case 0X00720716: return 3439;
                case 0X00720717: return 3440;
                case 0X00720718: return 3441;
                default:         return -1;
            }

        case 0X0074:
            switch (tag) {
                case 0X00740120: return 3442;
                case 0X00740121: return 3443;
                case 0X00741000: return 3444;
                case 0X00741002: return 3445;
                case 0X00741004: return 3446;
                case 0X00741006: return 3447;
                case 0X00741007: return 3448;
                case 0X00741008: return 3449;
                case 0X0074100A: return 3450;
                case 0X0074100C: return 3451;
                case 0X0074100E: return 3452;
                case 0X00741020: return 3453;
                case 0X00741022: return 3454;
                case 0X00741024: return 3455;
                case 0X00741025: return 3456;
                case 0X00741026: return 3457;
                case 0X00741027: return 3458;
                case 0X00741028: return 3459;
                case 0X0074102A: return 3460;
                case 0X0074102B: return 3461;
                case 0X0074102C: return 3462;
                case 0X0074102D: return 3463;
                case 0X00741030: return 3464;
                case 0X00741032: return 3465;
                case 0X00741034: return 3466;
                case 0X00741036: return 3467;
                case 0X00741038: return 3468;
                case 0X0074103A: return 3469;
                case 0X00741040: return 3470;
                case 0X00741042: return 3471;
                case 0X00741044: return 3472;
                case 0X00741046: return 3473;
                case 0X00741048: return 3474;
                case 0X0074104A: return 3475;
                case 0X0074104C: return 3476;
                case 0X0074104E: return 3477;
                case 0X00741050: return 3478;
                case 0X00741052: return 3479;
                case 0X00741054: return 3480;
                case 0X00741056: return 3481;
                case 0X00741057: return 3482;
                case 0X00741200: return 3483;
                case 0X00741202: return 3484;
                case 0X00741204: return 3485;
                case 0X00741210: return 3486;
                case 0X00741212: return 3487;
                case 0X00741216: return 3488;
                case 0X00741220: return 3489;
                case 0X00741222: return 3490;
                case 0X00741224: return 3491;
                case 0X00741230: return 3492;
                case 0X00741234: return 3493;
                case 0X00741236: return 3494;
                case 0X00741238: return 3495;
                case 0X00741242: return 3496;
                case 0X00741244: return 3497;
                case 0X00741246: return 3498;
                case 0X00741324: return 3499;
                case 0X00741338: return 3500;
                case 0X0074133A: return 3501;
                case 0X00741401: return 3502;
                case 0X00741402: return 3503;
                case 0X00741403: return 3504;
                case 0X00741404: return 3505;
                case 0X00741405: return 3506;
                case 0X00741406: return 3507;
                case 0X00741407: return 3508;
                case 0X00741408: return 3509;
                case 0X00741409: return 3510;
                case 0X0074140A: return 3511;
                case 0X0074140B: return 3512;
                case 0X0074140C: return 3513;
                case 0X0074140D: return 3514;
                case 0X0074140E: return 3515;
                default:         return -1;
            }

        case 0X0076:
            switch (tag) {
                case 0X00760001: return 3516;
                case 0X00760003: return 3517;
                case 0X00760006: return 3518;
                case 0X00760008: return 3519;
                case 0X0076000A: return 3520;
                case 0X0076000C: return 3521;
                case 0X0076000E: return 3522;
                case 0X00760010: return 3523;
                case 0X00760020: return 3524;
                case 0X00760030: return 3525;
                case 0X00760032: return 3526;
                case 0X00760034: return 3527;
                case 0X00760036: return 3528;
                case 0X00760038: return 3529;
                case 0X00760040: return 3530;
                case 0X00760055: return 3531;
                case 0X00760060: return 3532;
                case 0X00760070: return 3533;
                case 0X00760080: return 3534;
                case 0X00760090: return 3535;
                case 0X007600A0: return 3536;
                case 0X007600B0: return 3537;
                case 0X007600C0: return 3538;
                default:         return -1;
            }

        case 0X0078:
            switch (tag) {
                case 0X00780001: return 3539;
                case 0X00780010: return 3540;
                case 0X00780020: return 3541;
                case 0X00780024: return 3542;
                case 0X00780026: return 3543;
                case 0X00780028: return 3544;
                case 0X0078002A: return 3545;
                case 0X0078002E: return 3546;
                case 0X00780050: return 3547;
                case 0X00780060: return 3548;
                case 0X00780070: return 3549;
                case 0X00780090: return 3550;
                case 0X007800A0: return 3551;
                case 0X007800B0: return 3552;
                case 0X007800B2: return 3553;
                case 0X007800B4: return 3554;
                case 0X007800B6: return 3555;
                case 0X007800B8: return 3556;
                default:         return -1;
            }

        case 0X0080:
            switch (tag) {
                case 0X00800001: return 3557;
                case 0X00800002: return 3558;
                case 0X00800003: return 3559;
                case 0X00800004: return 3560;
                case 0X00800005: return 3561;
                case 0X00800006: return 3562;
                case 0X00800007: return 3563;
                case 0X00800008: return 3564;
                case 0X00800009: return 3565;
                case 0X00800010: return 3566;
                case 0X00800011: return 3567;
                case 0X00800012: return 3568;
                case 0X00800013: return 3569;
                default:         return -1;
            }

        case 0X0082:
            switch (tag) {
                case 0X00820001: return 3570;
                case 0X00820003: return 3571;
                case 0X00820004: return 3572;
                case 0X00820005: return 3573;
                case 0X00820006: return 3574;
                case 0X00820007: return 3575;
                case 0X00820008: return 3576;
                case 0X0082000A: return 3577;
                case 0X0082000C: return 3578;
                case 0X00820010: return 3579;
                case 0X00820016: return 3580;
                case 0X00820017: return 3581;
                case 0X00820018: return 3582;
                case 0X00820019: return 3583;
                case 0X00820021: return 3584;
                case 0X00820022: return 3585;
                case 0X00820023: return 3586;
                case 0X00820032: return 3587;
                case 0X00820033: return 3588;
                case 0X00820034: return 3589;
                case 0X00820035: return 3590;
                case 0X00820036: return 3591;
                case 0X00820037: return 3592;
                case 0X00820038: return 3593;
                default:         return -1;
            }

        case 0X0088:
            switch (tag) {
                case 0X00880130: return 3594;
                case 0X00880140: return 3595;
                case 0X00880200: return 3596;
                case 0X00880904: return 3597;
                case 0X00880906: return 3598;
                case 0X00880910: return 3599;
                case 0X00880912: return 3600;
                default:         return -1;
            }

        case 0X0100:
            switch (tag) {
                case 0X01000410: return 3601;
                case 0X01000420: return 3602;
                case 0X01000424: return 3603;
                case 0X01000426: return 3604;
                default:         return -1;
            }

        case 0X0400:
            switch (tag) {
                case 0X04000005: return 3605;
                case 0X04000010: return 3606;
                case 0X04000015: return 3607;
                case 0X04000020: return 3608;
                case 0X04000100: return 3609;
                case 0X04000105: return 3610;
                case 0X04000110: return 3611;
                case 0X04000115: return 3612;
                case 0X04000120: return 3613;
                case 0X04000305: return 3614;
                case 0X04000310: return 3615;
                case 0X04000315: return 3616;
                case 0X04000401: return 3617;
                case 0X04000402: return 3618;
                case 0X04000403: return 3619;
                case 0X04000404: return 3620;
                case 0X04000500: return 3621;
                case 0X04000510: return 3622;
                case 0X04000520: return 3623;
                case 0X04000550: return 3624;
                case 0X04000551: return 3625;
                case 0X04000552: return 3626;
                case 0X04000561: return 3627;
                case 0X04000562: return 3628;
                case 0X04000563: return 3629;
                case 0X04000564: return 3630;
                case 0X04000565: return 3631;
                case 0X04000600: return 3632;
                default:         return -1;
            }

        case 0X2000:
            switch (tag) {
                case 0X20000010: return 3633;
                case 0X2000001E: return 3634;
                case 0X20000020: return 3635;
                case 0X20000030: return 3636;
                case 0X20000040: return 3637;
                case 0X20000050: return 3638;
                case 0X20000060: return 3639;
                case 0X20000061: return 3640;
                case 0X20000062: return 3641;
                case 0X20000063: return 3642;
                case 0X20000065: return 3643;
                case 0X20000067: return 3644;
                case 0X20000069: return 3645;
                case 0X2000006A: return 3646;
                case 0X200000A0: return 3647;
                case 0X200000A1: return 3648;
                case 0X200000A2: return 3649;
                case 0X200000A4: return 3650;
                case 0X200000A8: return 3651;
                case 0X20000500: return 3652;
                case 0X20000510: return 3653;
                default:         return -1;
            }

        case 0X2010:
            switch (tag) {
                case 0X20100010: return 3654;
                case 0X20100030: return 3655;
                case 0X20100040: return 3656;
                case 0X20100050: return 3657;
                case 0X20100052: return 3658;
                case 0X20100054: return 3659;
                case 0X20100060: return 3660;
                case 0X20100080: return 3661;
                case 0X201000A6: return 3662;
                case 0X201000A7: return 3663;
                case 0X201000A8: return 3664;
                case 0X201000A9: return 3665;
                case 0X20100100: return 3666;
                case 0X20100110: return 3667;
                case 0X20100120: return 3668;
                case 0X20100130: return 3669;
                case 0X20100140: return 3670;
                case 0X20100150: return 3671;
                case 0X20100152: return 3672;
                case 0X20100154: return 3673;
                case 0X2010015E: return 3674;
                case 0X20100160: return 3675;
                case 0X20100376: return 3676;
                case 0X20100500: return 3677;
                case 0X20100510: return 3678;
                case 0X20100520: return 3679;
                default:         return -1;
            }

        case 0X2020:
            switch (tag) {
                case 0X20200010: return 3680;
                case 0X20200020: return 3681;
                case 0X20200030: return 3682;
                case 0X20200040: return 3683;
                case 0X20200050: return 3684;
                case 0X202000A0: return 3685;
                case 0X202000A2: return 3686;
                case 0X20200110: return 3687;
                case 0X20200111: return 3688;
                case 0X20200130: return 3689;
                case 0X20200140: return 3690;
                default:         return -1;
            }

        case 0X2030:
            switch (tag) {
                case 0X20300010: return 3691;
                case 0X20300020: return 3692;
                default:         return -1;
            }

        case 0X2040:
            switch (tag) {
                case 0X20400010: return 3693;
                case 0X20400011: return 3694;
                case 0X20400020: return 3695;
                case 0X20400060: return 3696;
                case 0X20400070: return 3697;
                case 0X20400072: return 3698;
                case 0X20400074: return 3699;
                case 0X20400080: return 3700;
                case 0X20400082: return 3701;
                case 0X20400090: return 3702;
                case 0X20400100: return 3703;
                case 0X20400500: return 3704;
                default:         return -1;
            }

        case 0X2050:
            switch (tag) {
                case 0X20500010: return 3705;
                case 0X20500020: return 3706;
                case 0X20500500: return 3707;
                default:         return -1;
            }

        case 0X2100:
            switch (tag) {
                case 0X21000010: return 3708;
                case 0X21000020: return 3709;
                case 0X21000030: return 3710;
                case 0X21000040: return 3711;
                case 0X21000050: return 3712;
                case 0X21000070: return 3713;
                case 0X21000140: return 3714;
                case 0X21000160: return 3715;
                case 0X21000170: return 3716;
                case 0X21000500: return 3717;
                default:         return -1;
            }

        case 0X2110:
            switch (tag) {
                case 0X21100010: return 3718;
                case 0X21100020: return 3719;
                case 0X21100030: return 3720;
                case 0X21100099: return 3721;
                default:         return -1;
            }

        case 0X2120:
            switch (tag) {
                case 0X21200010: return 3722;
                case 0X21200050: return 3723;
                case 0X21200070: return 3724;
                default:         return -1;
            }

        case 0X2130:
            switch (tag) {
                case 0X21300010: return 3725;
                case 0X21300015: return 3726;
                case 0X21300030: return 3727;
                case 0X21300040: return 3728;
                case 0X21300050: return 3729;
                case 0X21300060: return 3730;
                case 0X21300080: return 3731;
                case 0X213000A0: return 3732;
                case 0X213000C0: return 3733;
                default:         return -1;
            }

        case 0X2200:
            switch (tag) {
                case 0X22000001: return 3734;
                case 0X22000002: return 3735;
                case 0X22000003: return 3736;
                case 0X22000004: return 3737;
                case 0X22000005: return 3738;
                case 0X22000006: return 3739;
                case 0X22000007: return 3740;
                case 0X22000008: return 3741;
                case 0X22000009: return 3742;
                case 0X2200000A: return 3743;
                case 0X2200000B: return 3744;
                case 0X2200000C: return 3745;
                case 0X2200000D: return 3746;
                case 0X2200000E: return 3747;
                case 0X2200000F: return 3748;
                case 0X22000020: return 3749;
                default:         return -1;
            }

        case 0X3002:
            switch (tag) {
                case 0X30020002: return 3750;
                case 0X30020003: return 3751;
                case 0X30020004: return 3752;
                case 0X3002000A: return 3753;
                case 0X3002000C: return 3754;
                case 0X3002000D: return 3755;
                case 0X3002000E: return 3756;
                case 0X30020010: return 3757;
                case 0X30020011: return 3758;
                case 0X30020012: return 3759;
                case 0X30020020: return 3760;
                case 0X30020022: return 3761;
                case 0X30020024: return 3762;
                case 0X30020026: return 3763;
                case 0X30020028: return 3764;
                case 0X30020029: return 3765;
                case 0X30020030: return 3766;
                case 0X30020032: return 3767;
                case 0X30020034: return 3768;
                case 0X30020040: return 3769;
                case 0X30020041: return 3770;
                case 0X30020042: return 3771;
                case 0X30020050: return 3772;
                case 0X30020051: return 3773;
                case 0X30020052: return 3774;
                default:         return -1;
            }

        case 0X3004:
            switch (tag) {
                case 0X30040001: return 3775;
                case 0X30040002: return 3776;
                case 0X30040004: return 3777;
                case 0X30040005: return 3778;
                case 0X30040006: return 3779;
                case 0X30040008: return 3780;
                case 0X3004000A: return 3781;
                case 0X3004000C: return 3782;
                case 0X3004000E: return 3783;
                case 0X30040010: return 3784;
                case 0X30040012: return 3785;
                case 0X30040014: return 3786;
                case 0X30040040: return 3787;
                case 0X30040042: return 3788;
                case 0X30040050: return 3789;
                case 0X30040052: return 3790;
                case 0X30040054: return 3791;
                case 0X30040056: return 3792;
                case 0X30040058: return 3793;
                case 0X30040060: return 3794;
                case 0X30040062: return 3795;
                case 0X30040070: return 3796;
                case 0X30040072: return 3797;
                case 0X30040074: return 3798;
                default:         return -1;
            }

        case 0X3006:
            switch (tag) {
                case 0X30060002: return 3799;
                case 0X30060004: return 3800;
                case 0X30060006: return 3801;
                case 0X30060008: return 3802;
                case 0X30060009: return 3803;
                case 0X30060010: return 3804;
                case 0X30060012: return 3805;
                case 0X30060014: return 3806;
                case 0X30060016: return 3807;
                case 0X30060018: return 3808;
                case 0X30060020: return 3809;
                case 0X30060022: return 3810;
                case 0X30060024: return 3811;
                case 0X30060026: return 3812;
                case 0X30060028: return 3813;
                case 0X3006002A: return 3814;
                case 0X3006002C: return 3815;
                case 0X30060030: return 3816;
                case 0X30060033: return 3817;
                case 0X30060036: return 3818;
                case 0X30060037: return 3819;
                case 0X30060038: return 3820;
                case 0X30060039: return 3821;
                case 0X30060040: return 3822;
                case 0X30060042: return 3823;
                case 0X30060044: return 3824;
                case 0X30060045: return 3825;
                case 0X30060046: return 3826;
                case 0X30060048: return 3827;
                case 0X30060049: return 3828;
                case 0X3006004A: return 3829;
                case 0X30060050: return 3830;
                case 0X30060080: return 3831;
                case 0X30060082: return 3832;
                case 0X30060084: return 3833;
                case 0X30060085: return 3834;
                case 0X30060086: return 3835;
                case 0X30060088: return 3836;
                case 0X300600A0: return 3837;
                case 0X300600A4: return 3838;
                case 0X300600A6: return 3839;
                case 0X300600B0: return 3840;
                case 0X300600B2: return 3841;
                case 0X300600B4: return 3842;
                case 0X300600B6: return 3843;
                case 0X300600B7: return 3844;
                case 0X300600B8: return 3845;
                case 0X300600B9: return 3846;
                case 0X300600C0: return 3847;
                case 0X300600C2: return 3848;
                case 0X300600C4: return 3849;
                case 0X300600C6: return 3850;
                case 0X300600C8: return 3851;
                case 0X300600C9: return 3852;
                case 0X300600CA: return 3853;
                case 0X300600CB: return 3854;
                default:         return -1;
            }

        case 0X3008:
            switch (tag) {
                case 0X30080010: return 3855;
                case 0X30080012: return 3856;
                case 0X30080014: return 3857;
                case 0X30080016: return 3858;
                case 0X30080020: return 3859;
                case 0X30080021: return 3860;
                case 0X30080022: return 3861;
                case 0X30080024: return 3862;
                case 0X30080025: return 3863;
                case 0X3008002A: return 3864;
                case 0X3008002B: return 3865;
                case 0X3008002C: return 3866;
                case 0X30080030: return 3867;
                case 0X30080032: return 3868;
                case 0X30080033: return 3869;
                case 0X30080036: return 3870;
                case 0X30080037: return 3871;
                case 0X3008003A: return 3872;
                case 0X3008003B: return 3873;
                case 0X30080040: return 3874;
                case 0X30080041: return 3875;
                case 0X30080042: return 3876;
                case 0X30080044: return 3877;
                case 0X30080045: return 3878;
                case 0X30080046: return 3879;
                case 0X30080047: return 3880;
                case 0X30080048: return 3881;
                case 0X30080050: return 3882;
                case 0X30080052: return 3883;
                case 0X30080054: return 3884;
                case 0X30080056: return 3885;
                case 0X3008005A: return 3886;
                case 0X30080060: return 3887;
                case 0X30080061: return 3888;
                case 0X30080062: return 3889;
                case 0X30080063: return 3890;
                case 0X30080064: return 3891;
                case 0X30080065: return 3892;
                case 0X30080066: return 3893;
                case 0X30080067: return 3894;
                case 0X30080068: return 3895;
                case 0X3008006A: return 3896;
                case 0X30080070: return 3897;
                case 0X30080072: return 3898;
                case 0X30080074: return 3899;
                case 0X30080076: return 3900;
                case 0X30080078: return 3901;
                case 0X3008007A: return 3902;
                case 0X30080080: return 3903;
                case 0X30080082: return 3904;
                case 0X30080090: return 3905;
                case 0X30080092: return 3906;
                case 0X300800A0: return 3907;
                case 0X300800B0: return 3908;
                case 0X300800C0: return 3909;
                case 0X300800D0: return 3910;
                case 0X300800E0: return 3911;
                case 0X300800F0: return 3912;
                case 0X300800F2: return 3913;
                case 0X300800F4: return 3914;
                case 0X300800F6: return 3915;
                case 0X30080100: return 3916;
                case 0X30080105: return 3917;
                case 0X30080110: return 3918;
                case 0X30080116: return 3919;
                case 0X30080120: return 3920;
                case 0X30080122: return 3921;
                case 0X30080130: return 3922;
                case 0X30080132: return 3923;
                case 0X30080134: return 3924;
                case 0X30080136: return 3925;
                case 0X30080138: return 3926;
                case 0X3008013A: return 3927;
                case 0X3008013C: return 3928;
                case 0X30080140: return 3929;
                case 0X30080142: return 3930;
                case 0X30080150: return 3931;
                case 0X30080152: return 3932;
                case 0X30080160: return 3933;
                case 0X30080162: return 3934;
                case 0X30080164: return 3935;
                case 0X30080166: return 3936;
                case 0X30080168: return 3937;
                case 0X30080171: return 3938;
                case 0X30080172: return 3939;
                case 0X30080173: return 3940;
                case 0X30080200: return 3941;
                case 0X30080202: return 3942;
                case 0X30080220: return 3943;
                case 0X30080223: return 3944;
                case 0X30080224: return 3945;
                case 0X30080230: return 3946;
                case 0X30080240: return 3947;
                case 0X30080250: return 3948;
                case 0X30080251: return 3949;
                default:         return -1;
            }

        case 0X300A:
            switch (tag) {
                case 0X300A0002: return 3950;
                case 0X300A0003: return 3951;
                case 0X300A0004: return 3952;
                case 0X300A0006: return 3953;
                case 0X300A0007: return 3954;
                case 0X300A0009: return 3955;
                case 0X300A000A: return 3956;
                case 0X300A000B: return 3957;
                case 0X300A000C: return 3958;
                case 0X300A000E: return 3959;
                case 0X300A0010: return 3960;
                case 0X300A0012: return 3961;
                case 0X300A0013: return 3962;
                case 0X300A0014: return 3963;
                case 0X300A0015: return 3964;
                case 0X300A0016: return 3965;
                case 0X300A0018: return 3966;
                case 0X300A001A: return 3967;
                case 0X300A0020: return 3968;
                case 0X300A0021: return 3969;
                case 0X300A0022: return 3970;
                case 0X300A0023: return 3971;
                case 0X300A0025: return 3972;
                case 0X300A0026: return 3973;
                case 0X300A0027: return 3974;
                case 0X300A0028: return 3975;
                case 0X300A002A: return 3976;
                case 0X300A002B: return 3977;
                case 0X300A002C: return 3978;
                case 0X300A002D: return 3979;
                case 0X300A0040: return 3980;
                case 0X300A0042: return 3981;
                case 0X300A0043: return 3982;
                case 0X300A0044: return 3983;
                case 0X300A0046: return 3984;
                case 0X300A0048: return 3985;
                case 0X300A004A: return 3986;
                case 0X300A004B: return 3987;
                case 0X300A004C: return 3988;
                case 0X300A004E: return 3989;
                case 0X300A004F: return 3990;
                case 0X300A0050: return 3991;
                case 0X300A0051: return 3992;
                case 0X300A0052: return 3993;
                case 0X300A0053: return 3994;
                case 0X300A0055: return 3995;
                case 0X300A0070: return 3996;
                case 0X300A0071: return 3997;
                case 0X300A0072: return 3998;
                case 0X300A0078: return 3999;
                case 0X300A0079: return 4000;
                case 0X300A007A: return 4001;
                case 0X300A007B: return 4002;
                case 0X300A0080: return 4003;
                case 0X300A0082: return 4004;
                case 0X300A0083: return 4005;
                case 0X300A0084: return 4006;
                case 0X300A0086: return 4007;
                case 0X300A0088: return 4008;
                case 0X300A0089: return 4009;
                case 0X300A008A: return 4010;
                case 0X300A008B: return 4011;
                case 0X300A008C: return 4012;
                case 0X300A008D: return 4013;
                case 0X300A008E: return 4014;
                case 0X300A008F: return 4015;
                case 0X300A0090: return 4016;
                case 0X300A0091: return 4017;
                case 0X300A0092: return 4018;
                case 0X300A0093: return 4019;
                case 0X300A0094: return 4020;
                case 0X300A00A0: return 4021;
                case 0X300A00A2: return 4022;
                case 0X300A00A4: return 4023;
                case 0X300A00B0: return 4024;
                case 0X300A00B2: return 4025;
                case 0X300A00B3: return 4026;
                case 0X300A00B4: return 4027;
                case 0X300A00B6: return 4028;
                case 0X300A00B8: return 4029;
                case 0X300A00BA: return 4030;
                case 0X300A00BB: return 4031;
                case 0X300A00BC: return 4032;
                case 0X300A00BE: return 4033;
                case 0X300A00C0: return 4034;
                case 0X300A00C2: return 4035;
                case 0X300A00C3: return 4036;
                case 0X300A00C4: return 4037;
                case 0X300A00C5: return 4038;
                case 0X300A00C6: return 4039;
                case 0X300A00C7: return 4040;
                case 0X300A00C8: return 4041;
                case 0X300A00CA: return 4042;
                case 0X300A00CC: return 4043;
                case 0X300A00CE: return 4044;
                case 0X300A00D0: return 4045;
                case 0X300A00D1: return 4046;
                case 0X300A00D2: return 4047;
                case 0X300A00D3: return 4048;
                case 0X300A00D4: return 4049;
                case 0X300A00D5: return 4050;
                case 0X300A00D6: return 4051;
                case 0X300A00D7: return 4052;
                case 0X300A00D8: return 4053;
                case 0X300A00D9: return 4054;
                case 0X300A00DA: return 4055;
                case 0X300A00DB: return 4056;
                case 0X300A00DC: return 4057;
                case 0X300A00DD: return 4058;
                case 0X300A00DE: return 4059;
                case 0X300A00E0: return 4060;
                case 0X300A00E1: return 4061;
                case 0X300A00E2: return 4062;
                case 0X300A00E3: return 4063;
                case 0X300A00E4: return 4064;
                case 0X300A00E5: return 4065;
                case 0X300A00E6: return 4066;
                case 0X300A00E7: return 4067;
                case 0X300A00E8: return 4068;
                case 0X300A00E9: return 4069;
                case 0X300A00EA: return 4070;
                case 0X300A00EB: return 4071;
                case 0X300A00EC: return 4072;
                case 0X300A00ED: return 4073;
                case 0X300A00EE: return 4074;
                case 0X300A00EF: return 4075;
                case 0X300A00F0: return 4076;
                case 0X300A00F2: return 4077;
                case 0X300A00F3: return 4078;
                case 0X300A00F4: return 4079;
                case 0X300A00F5: return 4080;
                case 0X300A00F6: return 4081;
                case 0X300A00F7: return 4082;
                case 0X300A00F8: return 4083;
                case 0X300A00F9: return 4084;
                case 0X300A00FA: return 4085;
                case 0X300A00FB: return 4086;
                case 0X300A00FC: return 4087;
                case 0X300A00FE: return 4088;
                case 0X300A0100: return 4089;
                case 0X300A0102: return 4090;
                case 0X300A0104: return 4091;
                case 0X300A0106: return 4092;
                case 0X300A0107: return 4093;
                case 0X300A0108: return 4094;
                case 0X300A0109: return 4095;
                case 0X300A010A: return 4096;
                case 0X300A010C: return 4097;
                case 0X300A010E: return 4098;
                case 0X300A0110: return 4099;
                case 0X300A0111: return 4100;
                case 0X300A0112: return 4101;
                case 0X300A0114: return 4102;
                case 0X300A0115: return 4103;
                case 0X300A0116: return 4104;
                case 0X300A0118: return 4105;
                case 0X300A011A: return 4106;
                case 0X300A011C: return 4107;
                case 0X300A011E: return 4108;
                case 0X300A011F: return 4109;
                case 0X300A0120: return 4110;
                case 0X300A0121: return 4111;
                case 0X300A0122: return 4112;
                case 0X300A0123: return 4113;
                case 0X300A0124: return 4114;
                case 0X300A0125: return 4115;
                case 0X300A0126: return 4116;
                case 0X300A0128: return 4117;
                case 0X300A0129: return 4118;
                case 0X300A012A: return 4119;
                case 0X300A012C: return 4120;
                case 0X300A012E: return 4121;
                case 0X300A0130: return 4122;
                case 0X300A0131: return 4123;
                case 0X300A0132: return 4124;
                case 0X300A0133: return 4125;
                case 0X300A0134: return 4126;
                case 0X300A0140: return 4127;
                case 0X300A0142: return 4128;
                case 0X300A0144: return 4129;
                case 0X300A0146: return 4130;
                case 0X300A0148: return 4131;
                case 0X300A014A: return 4132;
                case 0X300A014C: return 4133;
                case 0X300A014E: return 4134;
                case 0X300A0150: return 4135;
                case 0X300A0151: return 4136;
                case 0X300A0152: return 4137;
                case 0X300A0153: return 4138;
                case 0X300A0154: return 4139;
                case 0X300A0155: return 4140;
                case 0X300A0180: return 4141;
                case 0X300A0182: return 4142;
                case 0X300A0183: return 4143;
                case 0X300A0184: return 4144;
                case 0X300A0190: return 4145;
                case 0X300A0192: return 4146;
                case 0X300A0194: return 4147;
                case 0X300A0196: return 4148;
                case 0X300A0198: return 4149;
                case 0X300A0199: return 4150;
                case 0X300A019A: return 4151;
                case 0X300A01A0: return 4152;
                case 0X300A01A2: return 4153;
                case 0X300A01A4: return 4154;
                case 0X300A01A6: return 4155;
                case 0X300A01A8: return 4156;
                case 0X300A01B0: return 4157;
                case 0X300A01B2: return 4158;
                case 0X300A01B4: return 4159;
                case 0X300A01B6: return 4160;
                case 0X300A01B8: return 4161;
                case 0X300A01BA: return 4162;
                case 0X300A01BC: return 4163;
                case 0X300A01D0: return 4164;
                case 0X300A01D2: return 4165;
                case 0X300A01D4: return 4166;
                case 0X300A01D6: return 4167;
                case 0X300A0200: return 4168;
                case 0X300A0202: return 4169;
                case 0X300A0206: return 4170;
                case 0X300A0210: return 4171;
                case 0X300A0212: return 4172;
                case 0X300A0214: return 4173;
                case 0X300A0216: return 4174;
                case 0X300A0218: return 4175;
                case 0X300A021A: return 4176;
                case 0X300A021B: return 4177;
                case 0X300A021C: return 4178;
                case 0X300A0222: return 4179;
                case 0X300A0224: return 4180;
                case 0X300A0226: return 4181;
                case 0X300A0228: return 4182;
                case 0X300A0229: return 4183;
                case 0X300A022A: return 4184;
                case 0X300A022B: return 4185;
                case 0X300A022C: return 4186;
                case 0X300A022E: return 4187;
                case 0X300A0230: return 4188;
                case 0X300A0232: return 4189;
                case 0X300A0234: return 4190;
                case 0X300A0236: return 4191;
                case 0X300A0238: return 4192;
                case 0X300A0240: return 4193;
                case 0X300A0242: return 4194;
                case 0X300A0244: return 4195;
                case 0X300A0250: return 4196;
                case 0X300A0260: return 4197;
                case 0X300A0262: return 4198;
                case 0X300A0263: return 4199;
                case 0X300A0264: return 4200;
                case 0X300A0266: return 4201;
                case 0X300A026A: return 4202;
                case 0X300A026C: return 4203;
                case 0X300A0271: return 4204;
                case 0X300A0272: return 4205;
                case 0X300A0273: return 4206;
                case 0X300A0274: return 4207;
                case 0X300A0280: return 4208;
                case 0X300A0282: return 4209;
                case 0X300A0284: return 4210;
                case 0X300A0286: return 4211;
                case 0X300A0288: return 4212;
                case 0X300A028A: return 4213;
                case 0X300A028C: return 4214;
                case 0X300A0290: return 4215;
                case 0X300A0291: return 4216;
                case 0X300A0292: return 4217;
                case 0X300A0294: return 4218;
                case 0X300A0296: return 4219;
                case 0X300A0298: return 4220;
                case 0X300A029C: return 4221;
                case 0X300A029E: return 4222;
                case 0X300A02A0: return 4223;
                case 0X300A02A1: return 4224;
                case 0X300A02A2: return 4225;
                case 0X300A02A4: return 4226;
                case 0X300A02B0: return 4227;
                case 0X300A02B2: return 4228;
                case 0X300A02B3: return 4229;
                case 0X300A02B4: return 4230;
                case 0X300A02B8: return 4231;
                case 0X300A02BA: return 4232;
                case 0X300A02C8: return 4233;
                case 0X300A02D0: return 4234;
                case 0X300A02D2: return 4235;
                case 0X300A02D4: return 4236;
                case 0X300A02D6: return 4237;
                case 0X300A02E0: return 4238;
                case 0X300A02E1: return 4239;
                case 0X300A02E2: return 4240;
                case 0X300A02E3: return 4241;
                case 0X300A02E4: return 4242;
                case 0X300A02E5: return 4243;
                case 0X300A02E6: return 4244;
                case 0X300A02E7: return 4245;
                case 0X300A02E8: return 4246;
                case 0X300A02EA: return 4247;
                case 0X300A02EB: return 4248;
                case 0X300A0302: return 4249;
                case 0X300A0304: return 4250;
                case 0X300A0306: return 4251;
                case 0X300A0308: return 4252;
                case 0X300A0309: return 4253;
                case 0X300A030A: return 4254;
                case 0X300A030C: return 4255;
                case 0X300A030D: return 4256;
                case 0X300A030F: return 4257;
                case 0X300A0312: return 4258;
                case 0X300A0314: return 4259;
                case 0X300A0316: return 4260;
                case 0X300A0318: return 4261;
                case 0X300A0320: return 4262;
                case 0X300A0322: return 4263;
                case 0X300A0330: return 4264;
                case 0X300A0332: return 4265;
                case 0X300A0334: return 4266;
                case 0X300A0336: return 4267;
                case 0X300A0338: return 4268;
                case 0X300A033A: return 4269;
                case 0X300A033C: return 4270;
                case 0X300A0340: return 4271;
                case 0X300A0342: return 4272;
                case 0X300A0344: return 4273;
                case 0X300A0346: return 4274;
                case 0X300A0348: return 4275;
                case 0X300A034A: return 4276;
                case 0X300A034C: return 4277;
                case 0X300A0350: return 4278;
                case 0X300A0352: return 4279;
                case 0X300A0354: return 4280;
                case 0X300A0355: return 4281;
                case 0X300A0356: return 4282;
                case 0X300A0358: return 4283;
                case 0X300A035A: return 4284;
                case 0X300A0360: return 4285;
                case 0X300A0362: return 4286;
                case 0X300A0364: return 4287;
                case 0X300A0366: return 4288;
                case 0X300A0370: return 4289;
                case 0X300A0372: return 4290;
                case 0X300A0374: return 4291;
                case 0X300A0380: return 4292;
                case 0X300A0382: return 4293;
                case 0X300A0384: return 4294;
                case 0X300A0386: return 4295;
                case 0X300A0388: return 4296;
                case 0X300A038A: return 4297;
                case 0X300A038F: return 4298;
                case 0X300A0390: return 4299;
                case 0X300A0391: return 4300;
                case 0X300A0392: return 4301;
                case 0X300A0393: return 4302;
                case 0X300A0394: return 4303;
                case 0X300A0395: return 4304;
                case 0X300A0396: return 4305;
                case 0X300A0398: return 4306;
                case 0X300A0399: return 4307;
                case 0X300A039A: return 4308;
                case 0X300A03A0: return 4309;
                case 0X300A03A2: return 4310;
                case 0X300A03A4: return 4311;
                case 0X300A03A6: return 4312;
                case 0X300A03A8: return 4313;
                case 0X300A03AA: return 4314;
                case 0X300A03AC: return 4315;
                case 0X300A0401: return 4316;
                case 0X300A0402: return 4317;
                case 0X300A0410: return 4318;
                case 0X300A0412: return 4319;
                case 0X300A0420: return 4320;
                case 0X300A0421: return 4321;
                case 0X300A0422: return 4322;
                case 0X300A0423: return 4323;
                case 0X300A0424: return 4324;
                case 0X300A0425: return 4325;
                case 0X300A0426: return 4326;
                case 0X300A0431: return 4327;
                case 0X300A0432: return 4328;
                case 0X300A0433: return 4329;
                case 0X300A0434: return 4330;
                case 0X300A0435: return 4331;
                case 0X300A0436: return 4332;
                case 0X300A0440: return 4333;
                case 0X300A0441: return 4334;
                case 0X300A0442: return 4335;
                case 0X300A0443: return 4336;
                case 0X300A0450: return 4337;
                case 0X300A0451: return 4338;
                case 0X300A0452: return 4339;
                case 0X300A0453: return 4340;
                case 0X300A0501: return 4341;
                case 0X300A0502: return 4342;
                case 0X300A0503: return 4343;
                case 0X300A0504: return 4344;
                case 0X300A0505: return 4345;
                case 0X300A0506: return 4346;
                case 0X300A0507: return 4347;
                case 0X300A0508: return 4348;
                case 0X300A0509: return 4349;
                case 0X300A0510: return 4350;
                case 0X300A0511: return 4351;
                case 0X300A0512: return 4352;
                case 0X300A0600: return 4353;
                case 0X300A0601: return 4354;
                case 0X300A0602: return 4355;
                case 0X300A0603: return 4356;
                case 0X300A0604: return 4357;
                case 0X300A0605: return 4358;
                case 0X300A0606: return 4359;
                case 0X300A0607: return 4360;
                case 0X300A0608: return 4361;
                case 0X300A0609: return 4362;
                case 0X300A060A: return 4363;
                case 0X300A060B: return 4364;
                case 0X300A060C: return 4365;
                case 0X300A060D: return 4366;
                case 0X300A060E: return 4367;
                case 0X300A060F: return 4368;
                case 0X300A0610: return 4369;
                case 0X300A0611: return 4370;
                case 0X300A0612: return 4371;
                case 0X300A0613: return 4372;
                case 0X300A0614: return 4373;
                case 0X300A0615: return 4374;
                case 0X300A0616: return 4375;
                case 0X300A0617: return 4376;
                case 0X300A0618: return 4377;
                case 0X300A0619: return 4378;
                case 0X300A061A: return 4379;
                case 0X300A061B: return 4380;
                case 0X300A061C: return 4381;
                case 0X300A061D: return 4382;
                case 0X300A061E: return 4383;
                case 0X300A061F: return 4384;
                case 0X300A0620: return 4385;
                case 0X300A0621: return 4386;
                case 0X300A0622: return 4387;
                case 0X300A0623: return 4388;
                case 0X300A0624: return 4389;
                case 0X300A0625: return 4390;
                case 0X300A0626: return 4391;
                case 0X300A0627: return 4392;
                case 0X300A0628: return 4393;
                case 0X300A0629: return 4394;
                case 0X300A062A: return 4395;
                case 0X300A062B: return 4396;
                case 0X300A062C: return 4397;
                case 0X300A062D: return 4398;
                case 0X300A062E: return 4399;
                case 0X300A062F: return 4400;
                case 0X300A0630: return 4401;
                case 0X300A0631: return 4402;
                case 0X300A0632: return 4403;
                case 0X300A0634: return 4404;
                case 0X300A0635: return 4405;
                case 0X300A0636: return 4406;
                case 0X300A0637: return 4407;
                case 0X300A0638: return 4408;
                case 0X300A0639: return 4409;
                case 0X300A063A: return 4410;
                case 0X300A063B: return 4411;
                case 0X300A063C: return 4412;
                case 0X300A063D: return 4413;
                case 0X300A063E: return 4414;
                case 0X300A063F: return 4415;
                case 0X300A0640: return 4416;
                case 0X300A0641: return 4417;
                case 0X300A0642: return 4418;
                case 0X300A0643: return 4419;
                case 0X300A0644: return 4420;
                case 0X300A0645: return 4421;
                case 0X300A0646: return 4422;
                case 0X300A0647: return 4423;
                case 0X300A0648: return 4424;
                case 0X300A0649: return 4425;
                case 0X300A064A: return 4426;
                case 0X300A064B: return 4427;
                case 0X300A064C: return 4428;
                case 0X300A064D: return 4429;
                case 0X300A064E: return 4430;
                case 0X300A064F: return 4431;
                case 0X300A0650: return 4432;
                case 0X300A0651: return 4433;
                case 0X300A0652: return 4434;
                case 0X300A0653: return 4435;
                case 0X300A0654: return 4436;
                case 0X300A0655: return 4437;
                case 0X300A0656: return 4438;
                case 0X300A0657: return 4439;
                case 0X300A0658: return 4440;
                case 0X300A0659: return 4441;
                case 0X300A065A: return 4442;
                case 0X300A065B: return 4443;
                case 0X300A065C: return 4444;
                case 0X300A065D: return 4445;
                case 0X300A065E: return 4446;
                case 0X300A065F: return 4447;
                case 0X300A0660: return 4448;
                case 0X300A0661: return 4449;
                case 0X300A0662: return 4450;
                case 0X300A0663: return 4451;
                case 0X300A0664: return 4452;
                case 0X300A0665: return 4453;
                case 0X300A0666: return 4454;
                case 0X300A0667: return 4455;
                case 0X300A0668: return 4456;
                case 0X300A0669: return 4457;
                case 0X300A066A: return 4458;
                case 0X300A066B: return 4459;
                case 0X300A066C: return 4460;
                case 0X300A066D: return 4461;
                case 0X300A066E: return 4462;
                case 0X300A066F: return 4463;
                case 0X300A0670: return 4464;
                case 0X300A0671: return 4465;
                case 0X300A0672: return 4466;
                case 0X300A0673: return 4467;
                case 0X300A0674: return 4468;
                case 0X300A0675: return 4469;
                case 0X300A0676: return 4470;
                case 0X300A0677: return 4471;
                case 0X300A0678: return 4472;
                case 0X300A0679: return 4473;
                case 0X300A067A: return 4474;
                case 0X300A067B: return 4475;
                case 0X300A067C: return 4476;
                case 0X300A067D: return 4477;
                case 0X300A067E: return 4478;
                case 0X300A067F: return 4479;
                case 0X300A0680: return 4480;
                case 0X300A0681: return 4481;
                case 0X300A0682: return 4482;
                case 0X300A0683: return 4483;
                case 0X300A0684: return 4484;
                case 0X300A0685: return 4485;
                case 0X300A0686: return 4486;
                case 0X300A0687: return 4487;
                case 0X300A0688: return 4488;
                case 0X300A0689: return 4489;
                case 0X300A068A: return 4490;
                case 0X300A0700: return 4491;
                case 0X300A0701: return 4492;
                case 0X300A0702: return 4493;
                case 0X300A0703: return 4494;
                case 0X300A0704: return 4495;
                case 0X300A0705: return 4496;
                case 0X300A0706: return 4497;
                case 0X300A0707: return 4498;
                case 0X300A0708: return 4499;
                case 0X300A0709: return 4500;
                case 0X300A0714: return 4501;
                case 0X300A0715: return 4502;
                case 0X300A0716: return 4503;
                case 0X300A0722: return 4504;
                case 0X300A0723: return 4505;
                case 0X300A0730: return 4506;
                case 0X300A0731: return 4507;
                case 0X300A0732: return 4508;
                case 0X300A0733: return 4509;
                case 0X300A0734: return 4510;
                case 0X300A0735: return 4511;
                case 0X300A0736: return 4512;
                case 0X300A073A: return 4513;
                case 0X300A073B: return 4514;
                case 0X300A073E: return 4515;
                case 0X300A073F: return 4516;
                case 0X300A0740: return 4517;
                case 0X300A0741: return 4518;
                case 0X300A0742: return 4519;
                case 0X300A0743: return 4520;
                case 0X300A0744: return 4521;
                case 0X300A0745: return 4522;
                case 0X300A0746: return 4523;
                case 0X300A0760: return 4524;
                case 0X300A0761: return 4525;
                case 0X300A0762: return 4526;
                case 0X300A0772: return 4527;
                case 0X300A0773: return 4528;
                case 0X300A0774: return 4529;
                case 0X300A0780: return 4530;
                case 0X300A0782: return 4531;
                case 0X300A0783: return 4532;
                case 0X300A0784: return 4533;
                case 0X300A0785: return 4534;
                case 0X300A0786: return 4535;
                case 0X300A0787: return 4536;
                case 0X300A0788: return 4537;
                case 0X300A0789: return 4538;
                case 0X300A078A: return 4539;
                case 0X300A078B: return 4540;
                case 0X300A078C: return 4541;
                case 0X300A078D: return 4542;
                case 0X300A078E: return 4543;
                case 0X300A078F: return 4544;
                case 0X300A0790: return 4545;
                case 0X300A0791: return 4546;
                case 0X300A0792: return 4547;
                case 0X300A0793: return 4548;
                case 0X300A0794: return 4549;
                case 0X300A0795: return 4550;
                case 0X300A0796: return 4551;
                case 0X300A0797: return 4552;
                case 0X300A0798: return 4553;
                case 0X300A0799: return 4554;
                case 0X300A079A: return 4555;
                case 0X300A079B: return 4556;
                case 0X300A079C: return 4557;
                case 0X300A079D: return 4558;
                case 0X300A079E: return 4559;
                default:         return -1;
            }

        case 0X300C:
            switch (tag) {
                case 0X300C0002: return 4560;
                case 0X300C0004: return 4561;
                case 0X300C0006: return 4562;
                case 0X300C0007: return 4563;
                case 0X300C0008: return 4564;
                case 0X300C0009: return 4565;
                case 0X300C000A: return 4566;
                case 0X300C000C: return 4567;
                case 0X300C000E: return 4568;
                case 0X300C0020: return 4569;
                case 0X300C0022: return 4570;
                case 0X300C0040: return 4571;
                case 0X300C0042: return 4572;
                case 0X300C0050: return 4573;
                case 0X300C0051: return 4574;
                case 0X300C0055: return 4575;
                case 0X300C0060: return 4576;
                case 0X300C006A: return 4577;
                case 0X300C0080: return 4578;
                case 0X300C00A0: return 4579;
                case 0X300C00B0: return 4580;
                case 0X300C00C0: return 4581;
                case 0X300C00D0: return 4582;
                case 0X300C00E0: return 4583;
                case 0X300C00F0: return 4584;
                case 0X300C00F2: return 4585;
                case 0X300C00F4: return 4586;
                case 0X300C00F6: return 4587;
                case 0X300C0100: return 4588;
                case 0X300C0102: return 4589;
                case 0X300C0104: return 4590;
                case 0X300C0111: return 4591;
                case 0X300C0112: return 4592;
                case 0X300C0113: return 4593;
                default:         return -1;
            }

        case 0X300E:
            switch (tag) {
                case 0X300E0002: return 4594;
                case 0X300E0004: return 4595;
                case 0X300E0005: return 4596;
                case 0X300E0008: return 4597;
                default:         return -1;
            }

        case 0X3010:
            switch (tag) {
                case 0X30100001: return 4598;
                case 0X30100002: return 4599;
                case 0X30100003: return 4600;
                case 0X30100004: return 4601;
                case 0X30100005: return 4602;
                case 0X30100006: return 4603;
                case 0X30100007: return 4604;
                case 0X30100008: return 4605;
                case 0X30100009: return 4606;
                case 0X3010000A: return 4607;
                case 0X3010000B: return 4608;
                case 0X3010000C: return 4609;
                case 0X3010000D: return 4610;
                case 0X3010000E: return 4611;
                case 0X3010000F: return 4612;
                case 0X30100010: return 4613;
                case 0X30100011: return 4614;
                case 0X30100012: return 4615;
                case 0X30100013: return 4616;
                case 0X30100014: return 4617;
                case 0X30100015: return 4618;
                case 0X30100016: return 4619;
                case 0X30100017: return 4620;
                case 0X30100018: return 4621;
                case 0X30100019: return 4622;
                case 0X3010001A: return 4623;
                case 0X3010001B: return 4624;
                case 0X3010001C: return 4625;
                case 0X3010001D: return 4626;
                case 0X3010001E: return 4627;
                case 0X3010001F: return 4628;
                case 0X30100020: return 4629;
                case 0X30100021: return 4630;
                case 0X30100022: return 4631;
                case 0X30100023: return 4632;
                case 0X30100024: return 4633;
                case 0X30100025: return 4634;
                case 0X30100026: return 4635;
                case 0X30100027: return 4636;
                case 0X30100028: return 4637;
                case 0X30100029: return 4638;
                case 0X3010002A: return 4639;
                case 0X3010002B: return 4640;
                case 0X3010002C: return 4641;
                case 0X3010002D: return 4642;
                case 0X3010002E: return 4643;
                case 0X3010002F: return 4644;
                case 0X30100030: return 4645;
                case 0X30100031: return 4646;
                case 0X30100032: return 4647;
                case 0X30100033: return 4648;
                case 0X30100034: return 4649;
                case 0X30100035: return 4650;
                case 0X30100036: return 4651;
                case 0X30100037: return 4652;
                case 0X30100038: return 4653;
                case 0X30100039: return 4654;
                case 0X3010003A: return 4655;
                case 0X3010003B: return 4656;
                case 0X3010003C: return 4657;
                case 0X3010003D: return 4658;
                case 0X3010003E: return 4659;
                case 0X3010003F: return 4660;
                case 0X30100040: return 4661;
                case 0X30100041: return 4662;
                case 0X30100042: return 4663;
                case 0X30100043: return 4664;
                case 0X30100044: return 4665;
                case 0X30100045: return 4666;
                case 0X30100046: return 4667;
                case 0X30100047: return 4668;
                case 0X30100048: return 4669;
                case 0X30100049: return 4670;
                case 0X3010004A: return 4671;
                case 0X3010004B: return 4672;
                case 0X3010004C: return 4673;
                case 0X3010004D: return 4674;
                case 0X3010004E: return 4675;
                case 0X3010004F: return 4676;
                case 0X30100050: return 4677;
                case 0X30100051: return 4678;
                case 0X30100052: return 4679;
                case 0X30100053: return 4680;
                case 0X30100054: return 4681;
                case 0X30100055: return 4682;
                case 0X30100056: return 4683;
                case 0X30100057: return 4684;
                case 0X30100058: return 4685;
                case 0X30100059: return 4686;
                case 0X3010005A: return 4687;
                case 0X3010005B: return 4688;
                case 0X3010005C: return 4689;
                case 0X3010005D: return 4690;
                case 0X3010005E: return 4691;
                case 0X3010005F: return 4692;
                case 0X30100060: return 4693;
                case 0X30100061: return 4694;
                case 0X30100062: return 4695;
                case 0X30100063: return 4696;
                case 0X30100064: return 4697;
                case 0X30100065: return 4698;
                case 0X30100066: return 4699;
                case 0X30100067: return 4700;
                case 0X30100068: return 4701;
                case 0X30100069: return 4702;
                case 0X3010006A: return 4703;
                case 0X3010006B: return 4704;
                case 0X3010006C: return 4705;
                case 0X3010006D: return 4706;
                case 0X3010006E: return 4707;
                case 0X3010006F: return 4708;
                case 0X30100070: return 4709;
                case 0X30100071: return 4710;
                case 0X30100073: return 4711;
                case 0X30100074: return 4712;
                case 0X30100075: return 4713;
                case 0X30100076: return 4714;
                case 0X30100077: return 4715;
                case 0X30100078: return 4716;
                case 0X30100079: return 4717;
                case 0X3010007A: return 4718;
                case 0X3010007B: return 4719;
                case 0X3010007C: return 4720;
                case 0X3010007D: return 4721;
                case 0X3010007E: return 4722;
                case 0X3010007F: return 4723;
                case 0X30100080: return 4724;
                case 0X30100081: return 4725;
                case 0X30100082: return 4726;
                case 0X30100083: return 4727;
                case 0X30100084: return 4728;
                case 0X30100085: return 4729;
                case 0X30100086: return 4730;
                case 0X30100087: return 4731;
                case 0X30100088: return 4732;
                case 0X30100089: return 4733;
                case 0X30100090: return 4734;
                case 0X30100091: return 4735;
                case 0X30100092: return 4736;
                case 0X30100093: return 4737;
                case 0X30100094: return 4738;
                case 0X30100095: return 4739;
                case 0X30100096: return 4740;
                case 0X30100097: return 4741;
                case 0X30100098: return 4742;
                case 0X30100099: return 4743;
                case 0X3010009A: return 4744;
                default:         return -1;
            }

        case 0X4000:
            switch (tag) {
                case 0X40000010: return 4745;
                case 0X40004000: return 4746;
                default:         return -1;
            }

        case 0X4008:
            switch (tag) {
                case 0X40080040: return 4747;
                case 0X40080042: return 4748;
                case 0X40080050: return 4749;
                case 0X400800FF: return 4750;
                case 0X40080100: return 4751;
                case 0X40080101: return 4752;
                case 0X40080102: return 4753;
                case 0X40080103: return 4754;
                case 0X40080108: return 4755;
                case 0X40080109: return 4756;
                case 0X4008010A: return 4757;
                case 0X4008010B: return 4758;
                case 0X4008010C: return 4759;
                case 0X40080111: return 4760;
                case 0X40080112: return 4761;
                case 0X40080113: return 4762;
                case 0X40080114: return 4763;
                case 0X40080115: return 4764;
                case 0X40080117: return 4765;
                case 0X40080118: return 4766;
                case 0X40080119: return 4767;
                case 0X4008011A: return 4768;
                case 0X40080200: return 4769;
                case 0X40080202: return 4770;
                case 0X40080210: return 4771;
                case 0X40080212: return 4772;
                case 0X40080300: return 4773;
                case 0X40084000: return 4774;
                default:         return -1;
            }

        case 0X4010:
            switch (tag) {
                case 0X40100001: return 4775;
                case 0X40100002: return 4776;
                case 0X40100004: return 4777;
                case 0X40101001: return 4778;
                case 0X40101004: return 4779;
                case 0X40101005: return 4780;
                case 0X40101006: return 4781;
                case 0X40101007: return 4782;
                case 0X40101008: return 4783;
                case 0X40101009: return 4784;
                case 0X4010100A: return 4785;
                case 0X40101010: return 4786;
                case 0X40101011: return 4787;
                case 0X40101012: return 4788;
                case 0X40101013: return 4789;
                case 0X40101014: return 4790;
                case 0X40101015: return 4791;
                case 0X40101016: return 4792;
                case 0X40101017: return 4793;
                case 0X40101018: return 4794;
                case 0X40101019: return 4795;
                case 0X4010101A: return 4796;
                case 0X4010101B: return 4797;
                case 0X4010101C: return 4798;
                case 0X4010101D: return 4799;
                case 0X4010101E: return 4800;
                case 0X4010101F: return 4801;
                case 0X40101020: return 4802;
                case 0X40101021: return 4803;
                case 0X40101023: return 4804;
                case 0X40101024: return 4805;
                case 0X40101025: return 4806;
                case 0X40101026: return 4807;
                case 0X40101027: return 4808;
                case 0X40101028: return 4809;
                case 0X40101029: return 4810;
                case 0X4010102A: return 4811;
                case 0X4010102B: return 4812;
                case 0X40101031: return 4813;
                case 0X40101033: return 4814;
                case 0X40101034: return 4815;
                case 0X40101037: return 4816;
                case 0X40101038: return 4817;
                case 0X40101039: return 4818;
                case 0X4010103A: return 4819;
                case 0X40101041: return 4820;
                case 0X40101042: return 4821;
                case 0X40101043: return 4822;
                case 0X40101044: return 4823;
                case 0X40101045: return 4824;
                case 0X40101046: return 4825;
                case 0X40101047: return 4826;
                case 0X40101048: return 4827;
                case 0X40101051: return 4828;
                case 0X40101052: return 4829;
                case 0X40101053: return 4830;
                case 0X40101054: return 4831;
                case 0X40101055: return 4832;
                case 0X40101056: return 4833;
                case 0X40101058: return 4834;
                case 0X40101059: return 4835;
                case 0X40101060: return 4836;
                case 0X40101061: return 4837;
                case 0X40101062: return 4838;
                case 0X40101064: return 4839;
                case 0X40101067: return 4840;
                case 0X40101068: return 4841;
                case 0X40101069: return 4842;
                case 0X4010106C: return 4843;
                case 0X4010106D: return 4844;
                case 0X4010106E: return 4845;
                case 0X4010106F: return 4846;
                case 0X40101070: return 4847;
                case 0X40101071: return 4848;
                case 0X40101072: return 4849;
                case 0X40101073: return 4850;
                case 0X40101075: return 4851;
                case 0X40101076: return 4852;
                case 0X40101077: return 4853;
                case 0X40101078: return 4854;
                case 0X40101079: return 4855;
                case 0X4010107A: return 4856;
                case 0X4010107B: return 4857;
                case 0X4010107C: return 4858;
                case 0X4010107D: return 4859;
                case 0X4010107E: return 4860;
                default:         return -1;
            }

        case 0X4FFE:
            switch (tag) {
                case 0X4FFE0001: return 4861;
                default:         return -1;
            }

        case 0X5200:
            switch (tag) {
                case 0X52009229: return 4862;
                case 0X52009230: return 4863;
                default:         return -1;
            }

        case 0X5400:
            switch (tag) {
                case 0X54000100: return 4864;
                case 0X54000110: return 4865;
                case 0X54000112: return 4866;
                case 0X54001004: return 4867;
                case 0X54001006: return 4868;
                case 0X5400100A: return 4869;
                case 0X54001010: return 4870;
                default:         return -1;
            }

        case 0X5600:
            switch (tag) {
                case 0X56000010: return 4871;
                case 0X56000020: return 4872;
                default:         return -1;
            }

        case 0X7FE0:
            switch (tag) {
                case 0X7FE00001: return 4873;
                case 0X7FE00002: return 4874;
                case 0X7FE00008: return 4875;
                case 0X7FE00009: return 4876;
                case 0X7FE00010: return 4877;
                case 0X7FE00020: return 4878;
                case 0X7FE00030: return 4879;
                case 0X7FE00040: return 4880;
                default:         return -1;
            }

        case 0XFFFA:
            switch (tag) {
                case 0XFFFAFFFA: return 4881;
                default:         return -1;
            }

        case 0XFFFC:
            switch (tag) {
                case 0XFFFCFFFC: return 4882;
                default:         return -1;
            }

        default:
            return -1;
    }
}



bool dcm_is_public_tag(uint32_t tag)
{
    int index = hash(tag);
    if (index >= 0) {
        return true;
    }
    return false;
}


bool dcm_is_private_tag(uint32_t tag)
{
    uint16_t group_number = (uint16_t)(tag >> 16);
    if ((group_number % 2) != 0) {
        return true;
    }
    return false;
}


bool dcm_is_valid_tag(uint32_t tag)
{
    if (tag == 0x00000000) {
        return false;
    }
    bool is_public_tag = dcm_is_public_tag(tag);
    bool is_private_tag = dcm_is_private_tag(tag);
    if (is_public_tag || is_private_tag) {
        return true;
    }
    return false;
}


bool dcm_is_valid_vr(const char *vr)
{
    if (vr == NULL) {
        return false;
    }
    if (strlen(vr) != 2) {
        return false;
    }
    if (strcmp(vr, "AE") == 0 ||
        strcmp(vr, "AS") == 0 ||
        strcmp(vr, "AT") == 0 ||
        strcmp(vr, "CS") == 0 ||
        strcmp(vr, "DA") == 0 ||
        strcmp(vr, "DS") == 0 ||
        strcmp(vr, "DT") == 0 ||
        strcmp(vr, "FD") == 0 ||
        strcmp(vr, "FL") == 0 ||
        strcmp(vr, "IL") == 0 ||
        strcmp(vr, "IS") == 0 ||
        strcmp(vr, "LO") == 0 ||
        strcmp(vr, "LT") == 0 ||
        strcmp(vr, "OB") == 0 ||
        strcmp(vr, "OD") == 0 ||
        strcmp(vr, "OF") == 0 ||
        strcmp(vr, "OL") == 0 ||
        strcmp(vr, "OV") == 0 ||
        strcmp(vr, "OW") == 0 ||
        strcmp(vr, "PN") == 0 ||
        strcmp(vr, "SH") == 0 ||
        strcmp(vr, "SL") == 0 ||
        strcmp(vr, "SQ") == 0 ||
        strcmp(vr, "SS") == 0 ||
        strcmp(vr, "ST") == 0 ||
        strcmp(vr, "SV") == 0 ||
        strcmp(vr, "TM") == 0 ||
        strcmp(vr, "UI") == 0 ||
        strcmp(vr, "UL") == 0 ||
        strcmp(vr, "UN") == 0 ||
        strcmp(vr, "UR") == 0 ||
        strcmp(vr, "US") == 0 ||
        strcmp(vr, "UT") == 0 ||
        strcmp(vr, "UV") == 0) {
        return true;
    }
    return false;
}


const char *dcm_dict_lookup_vr(uint32_t tag)
{
    int index = hash(tag);
    if (index < 0) {
        dcm_log_critical("Lookup of VR for Attribute '%08x' failed", tag);
        exit(1);
    }
    if (tag != dictionary[index].tag) {
        dcm_log_critical("Lookup of VR for Attribute '%08x' failed", tag);
        exit(1);
    }
    return dictionary[index].vr;
}


const char *dcm_dict_lookup_keyword(uint32_t tag)
{
    int index = hash(tag);
    if (index < 0) {
        dcm_log_critical("Lookup of Keyword for Attribute '%08x' failed.", tag);
        exit(1);
    }
    if (tag != dictionary[index].tag) {
        dcm_log_critical("Lookup of Keyword for Attribute '%08x' failed.", tag);
        exit(1);
    }
    return dictionary[index].keyword;
}

