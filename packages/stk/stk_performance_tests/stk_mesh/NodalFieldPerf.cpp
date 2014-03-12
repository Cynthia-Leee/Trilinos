#include <gtest/gtest.h>
#include <stk_mesh/base/Field.hpp>
#include <stk_mesh/base/FieldBase.hpp>
#include <stk_mesh/base/FieldDataManager.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_topology/topology.hpp>
#include <stk_mesh/base/CoordinateSystems.hpp>
#include <stk_util/environment/CPUTime.hpp>

#include <mpi.h>                        // for MPI_COMM_WORLD, MPI_Comm, etc
#include <stk_io/StkMeshIoBroker.hpp>   // for StkMeshIoBroker
#include <stk_mesh/base/GetEntities.hpp>  // for count_entities

#include <vector>
#include <string>

extern int* STKUNIT_ARGC;
extern char** STKUNIT_ARGV;

namespace
{

double initial_value1[3] = {-1, 2, -0.3};

void createNodalVectorField(stk::mesh::MetaData& meshMetaData, const std::string &field_name)
{
    stk::mesh::Field<double, stk::mesh::Cartesian3d> &field1 = meshMetaData.declare_field<stk::mesh::Field<double, stk::mesh::Cartesian3d> >(stk::topology::NODE_RANK, field_name);
    stk::mesh::put_field_on_all_nodes_with_initial_value(field1, initial_value1);
}

void createNodalVectorFields(stk::mesh::MetaData& meshMetaData)
{
    createNodalVectorField(meshMetaData, "disp");
    createNodalVectorField(meshMetaData, "vel");
    createNodalVectorField(meshMetaData, "acc");
    createNodalVectorField(meshMetaData, "force");
    meshMetaData.commit();
}

std::string getOption(const std::string& option, const std::string defaultString="no")
{
    std::string returnValue = defaultString;
    if ( STKUNIT_ARGV != 0 )
    {
        for (int i=0;i<*STKUNIT_ARGC;i++)
        {
            std::string input_argv(STKUNIT_ARGV[i]);
            if ( option == input_argv )
            {
                if ( (i+1) < *STKUNIT_ARGC )
                {
                    returnValue = std::string(STKUNIT_ARGV[i+1]);
                }
                break;
            }
        }
    }
    return returnValue;
}

void createMetaAndBulkData(stk::io::StkMeshIoBroker &exodusFileReader, stk::mesh::FieldDataManager *fieldDataManager)
{
    std::string exodusFileName = getOption("-i", "NO_FILE_SPECIFIED");
    ASSERT_NE(exodusFileName,"NO_FILE_SPECIFIED");

    exodusFileReader.add_mesh_database(exodusFileName, stk::io::READ_MESH);

    std::cerr << "Starting To Read Mesh: " << exodusFileName << std::endl;

    exodusFileReader.create_input_mesh();
    stk::mesh::MetaData &stkMeshMetaData = exodusFileReader.meta_data();
    createNodalVectorFields(stkMeshMetaData);

    Teuchos::RCP<stk::mesh::BulkData> arg_bulk_data(new stk::mesh::BulkData(stkMeshMetaData, MPI_COMM_WORLD, false, NULL, fieldDataManager));
    exodusFileReader.set_bulk_data(arg_bulk_data);
    stk::mesh::BulkData& stkMeshBulkData = *arg_bulk_data;

    bool delay_field_data_allocation = true;
    exodusFileReader.populate_mesh(delay_field_data_allocation);
    exodusFileReader.populate_field_data();

    std::cerr << "Finished Reading Mesh: " << exodusFileName << std::endl;

    stk::mesh::Selector allEntities = stkMeshMetaData.universal_part();
    std::vector<unsigned> entityCounts;
    stk::mesh::count_entities(allEntities, stkMeshBulkData, entityCounts);
    size_t numElements = entityCounts[stk::topology::ELEMENT_RANK];
    size_t numNodes = entityCounts[stk::topology::NODE_RANK];

    std::cerr << "Number of elements in " << exodusFileName << " = " << numElements << std::endl;
    std::cerr << "Number of nodes in " << exodusFileName << " = " << numNodes << std::endl;
}

void timeFieldOperations(stk::mesh::MetaData &stkMeshMetaData, stk::mesh::BulkData &stkMeshBulkData, double alpha, double beta, double gamma)
{
    std::string numIterationsString = getOption("-numIter", "1");
    const int numIterations = std::atoi(numIterationsString.c_str());
    std::cerr << "Starting timer for " << numIterations << " iterations of nodal addition (vec4 = vec1 + vec2 + vec3) test." << std::endl;
    double startTime   = stk::cpu_time();

        stk::mesh::Field<double, stk::mesh::Cartesian3d> &disp_field  = *stkMeshMetaData.get_field<stk::mesh::Field<double, stk::mesh::Cartesian3d> >("disp");
        stk::mesh::Field<double, stk::mesh::Cartesian3d> &vel_field   = *stkMeshMetaData.get_field<stk::mesh::Field<double, stk::mesh::Cartesian3d> >("vel");
        stk::mesh::Field<double, stk::mesh::Cartesian3d> &acc_field   = *stkMeshMetaData.get_field<stk::mesh::Field<double, stk::mesh::Cartesian3d> >("acc");
        stk::mesh::Field<double, stk::mesh::Cartesian3d> &force_field = *stkMeshMetaData.get_field<stk::mesh::Field<double, stk::mesh::Cartesian3d> >("force");

        const stk::mesh::BucketVector& allNodeBuckets = stkMeshBulkData.buckets(stk::topology::NODE_RANK);
        const size_t numNodeBuckets = allNodeBuckets.size();

    for(int i=0; i<numIterations; i++)
    {
        for (size_t bucketIndex=0;bucketIndex<numNodeBuckets;++bucketIndex)
        {
            const stk::mesh::Bucket &bucket = *allNodeBuckets[bucketIndex];
            double *disp = stk::mesh::field_data(disp_field, bucket);
            double *vel  = stk::mesh::field_data(vel_field, bucket);
            double *acc  = stk::mesh::field_data(acc_field, bucket);
            double *force = stk::mesh::field_data(force_field, bucket);
            size_t bucketLoopEnd = 3 * bucket.size();
            for (size_t nodeIndex=0;nodeIndex<bucketLoopEnd;nodeIndex+=3)
            {
                force[nodeIndex+0] = alpha*disp[nodeIndex+0] + beta*vel[nodeIndex+0] + gamma*acc[nodeIndex+0];
                force[nodeIndex+1] = alpha*disp[nodeIndex+1] + beta*vel[nodeIndex+1] + gamma*acc[nodeIndex+1];
                force[nodeIndex+2] = alpha*disp[nodeIndex+2] + beta*vel[nodeIndex+2] + gamma*acc[nodeIndex+2];
            }
        }
    }
    double elapsedTime = stk::cpu_time()- startTime;
    std::cerr << "That took " << elapsedTime << " seconds." << std::endl;
}

void testGoldValues(stk::mesh::MetaData &stkMeshMetaData, stk::mesh::BulkData &stkMeshBulkData, double alpha, double beta, double gamma)
{
    double goldX=initial_value1[0]*(alpha+beta+gamma);
    double goldY=initial_value1[1]*(alpha+beta+gamma);
    double goldZ=initial_value1[2]*(alpha+beta+gamma);

    stk::mesh::Field<double, stk::mesh::Cartesian3d> &force_field  = *stkMeshMetaData.get_field<stk::mesh::Field<double, stk::mesh::Cartesian3d> >("force");
    double tol=1.0e-4;
    const stk::mesh::BucketVector& buckets = stkMeshBulkData.buckets(stk::topology::NODE_RANK);
    for (size_t bucketIndex=0;bucketIndex<buckets.size();++bucketIndex)
    {
        const stk::mesh::Bucket &bucket = *buckets[bucketIndex];
        double *force  = stk::mesh::field_data(force_field, bucket);
        for (size_t nodeIndex=0;nodeIndex<3*bucket.size();nodeIndex+=3)
        {
            EXPECT_NEAR(goldX, force[nodeIndex+0], tol);
            EXPECT_NEAR(goldY, force[nodeIndex+1], tol);
            EXPECT_NEAR(goldZ, force[nodeIndex+2], tol);
        }
    }
}

void test1ToNSumOfNodalFields(stk::mesh::ContiguousFieldDataManager *fieldDataManager)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    stk::io::StkMeshIoBroker exodusFileReader(communicator);

    createMetaAndBulkData(exodusFileReader, fieldDataManager);

    stk::mesh::MetaData &stkMeshMetaData = exodusFileReader.meta_data();
    stk::mesh::BulkData &stkMeshBulkData = exodusFileReader.bulk_data();

    const stk::mesh::BucketVector& buckets = stkMeshBulkData.buckets(stk::topology::NODE_RANK);
    std::cerr << "Number of node buckets: " << buckets.size() << std::endl;

    double alpha = -1.4;
    double beta  = 0.3333333;
    double gamma = 3.14159;

    std::string numIterationsString = getOption("-numIter", "1");
    const int numIterations = std::atoi(numIterationsString.c_str());
    std::cerr << "Starting timer for " << numIterations << " iterations of nodal addition (vec4 = vec1 + vec2 + vec3) test." << std::endl;
    double startTime   = stk::cpu_time();

        stk::mesh::Field<double, stk::mesh::Cartesian3d> &disp_field  = *stkMeshMetaData.get_field<stk::mesh::Field<double, stk::mesh::Cartesian3d> >("disp");
        stk::mesh::Field<double, stk::mesh::Cartesian3d> &vel_field   = *stkMeshMetaData.get_field<stk::mesh::Field<double, stk::mesh::Cartesian3d> >("vel");
        stk::mesh::Field<double, stk::mesh::Cartesian3d> &acc_field   = *stkMeshMetaData.get_field<stk::mesh::Field<double, stk::mesh::Cartesian3d> >("acc");
        stk::mesh::Field<double, stk::mesh::Cartesian3d> &force_field = *stkMeshMetaData.get_field<stk::mesh::Field<double, stk::mesh::Cartesian3d> >("force");

        const stk::mesh::BucketVector& allNodeBuckets = stkMeshBulkData.buckets(stk::topology::NODE_RANK);
        const stk::mesh::Bucket &bucket = *allNodeBuckets[0];
        double *disp = stk::mesh::field_data(disp_field, bucket);
        double *vel  = stk::mesh::field_data(vel_field, bucket);
        double *acc  = stk::mesh::field_data(acc_field, bucket);
        double *force = stk::mesh::field_data(force_field, bucket);

        size_t numBytesForField = fieldDataManager->get_num_bytes_allocated_on_field(disp_field.mesh_meta_data_ordinal());
        stk::mesh::FieldMetaDataVector& field_meta_data_vector = const_cast<stk::mesh::FieldMetaDataVector&>(disp_field.get_meta_data_for_field());
        int numBytesPerEntity = field_meta_data_vector[0].m_bytes_per_entity;
        size_t numNodes = numBytesForField / numBytesPerEntity;
        size_t nodeLoopEnd = 3 * numNodes;

    for(int i=0; i<numIterations; i++)
    {
        for (size_t nodeIndex=0;nodeIndex<nodeLoopEnd;nodeIndex+=3)
        {
            force[nodeIndex+0] = alpha*disp[nodeIndex+0] + beta*vel[nodeIndex+0] + gamma*acc[nodeIndex+0];
            force[nodeIndex+1] = alpha*disp[nodeIndex+1] + beta*vel[nodeIndex+1] + gamma*acc[nodeIndex+1];
            force[nodeIndex+2] = alpha*disp[nodeIndex+2] + beta*vel[nodeIndex+2] + gamma*acc[nodeIndex+2];
        }
    }
    double elapsedTime = stk::cpu_time()- startTime;
    std::cerr << "That took " << elapsedTime << " seconds." << std::endl;

    testGoldValues(stkMeshMetaData, stkMeshBulkData, alpha, beta, gamma);
}

void testSumOfNodalFields(stk::mesh::FieldDataManager *fieldDataManager)
{
    MPI_Comm communicator = MPI_COMM_WORLD;
    stk::io::StkMeshIoBroker exodusFileReader(communicator);

    createMetaAndBulkData(exodusFileReader, fieldDataManager);

    stk::mesh::MetaData &stkMeshMetaData = exodusFileReader.meta_data();
    stk::mesh::BulkData &stkMeshBulkData = exodusFileReader.bulk_data();

    const stk::mesh::BucketVector& buckets = stkMeshBulkData.buckets(stk::topology::NODE_RANK);
    std::cerr << "Number of node buckets: " << buckets.size() << std::endl;

    double alpha = -1.4;
    double beta  = 0.3333333;
    double gamma = 3.14159;

    timeFieldOperations(stkMeshMetaData, stkMeshBulkData, alpha, beta, gamma);

    testGoldValues(stkMeshMetaData, stkMeshBulkData, alpha, beta, gamma);
}

TEST(NodalFieldPerformance, addNodalFieldsDefaultFieldManager)
{
    const int weKnowThereAreFiveRanks = 5;
    stk::mesh::DefaultFieldDataManager fieldDataManager(weKnowThereAreFiveRanks);
    testSumOfNodalFields(&fieldDataManager);
}

TEST(NodalFieldPerformance, addNodalFieldsContiguousFieldManager)
{
    stk::mesh::ContiguousFieldDataManager fieldDataManager;
    testSumOfNodalFields(&fieldDataManager);
}

TEST(NodalFieldPerformance, addNodalFields1ToNOrderingContiguousFieldManager)
{
    stk::mesh::ContiguousFieldDataManager fieldDataManager;
    test1ToNSumOfNodalFields(&fieldDataManager);
}

}
