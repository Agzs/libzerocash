/** @file
 *****************************************************************************

 A test for Zerocash.

 *****************************************************************************
 * @author     This file is part of libzerocash, developed by the Zerocash
 *             project and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include <stdlib.h>
#include <iostream>
#include <string>
#include <cstring>

#define BOOST_TEST_MODULE zerocashTest
#include <boost/test/included/unit_test.hpp>

#include "timer.h"

#include "libzerocash/Zerocash.h"
#include "libzerocash/ZerocashParams.h"
#include "libzerocash/Address.h"
#include "libzerocash/CoinCommitment.h"
#include "libzerocash/Coin.h"
#include "libzerocash/IncrementalMerkleTree.h"
#include "libzerocash/MintTransaction.h"
#include "libzerocash/PourTransaction.h"
#include "libzerocash/PourInput.h"
#include "libzerocash/PourOutput.h"
#include "libzerocash/utils/util.h"

using namespace std;
using namespace libsnark;

#define TEST_TREE_DEPTH 20

vector<bool> convertIntToVector(uint64_t val) {
	vector<bool> ret;

	for(unsigned int i = 0; i < sizeof(val) * 8; ++i, val >>= 1) {
		ret.push_back(val & 0x01);
	}

	reverse(ret.begin(), ret.end());
	return ret;
}

BOOST_AUTO_TEST_CASE ( AddressTest ) {
    cout << "\nADDRESS TEST\n" << endl;

    libzerocash::timer_start("Address");
    libzerocash::Address newAddress = libzerocash::Address::CreateNewRandomAddress();
    libzerocash::timer_stop("Address");

    cout << "New address addr_pk size: " << newAddress.get_addr_pk_size() << "B" << endl;
    cout << "New address addr_sk size: " << newAddress.get_addr_sk_size() << "B" << endl;

    cout << "Successfully created an address.\n" << endl;

    bool result = true;

    BOOST_CHECK (result);
}

BOOST_AUTO_TEST_CASE ( PrintSizeTest ) {

#define vZC_ADDR_PK_SIZE    (ZC_A_PK_SIZE+ZC_PK_ENC_SIZE)
    cout << "Params in Zerocash.h " << endl;

    cout << "New address addr_pk size: " << ZC_ADDR_PK_SIZE << "B" << endl;
    cout << "New address addr_sk size: " << ZC_ADDR_SK_SIZE << "B" << endl;
    
    cout << "Default tree height: " << ZEROCASH_DEFAULT_TREE_SIZE << endl;

    cout << "Coin size: " << ZC_COIN_SIZE << "B" << endl;
    cout << "Mint tx size: " << ZC_TX_MINT_SIZE << "B" << endl;
    cout << "Pour tx size: " << ZC_TX_POUR_SIZE << "B" << endl;
    
    cout << "Proof size: " << ZC_POUR_PROOF_SIZE << "B" << endl;
}

BOOST_AUTO_TEST_CASE( SaveAndLoadKeysFromFiles ) {
    cout << "\nSaveAndLoadKeysFromFiles TEST\n" << endl;

    cout << "Creating Params...\n" << endl;

    libzerocash::timer_start("Param Generation");
    auto keypair = libzerocash::ZerocashParams::GenerateNewKeyPair(TEST_TREE_DEPTH);
    libzerocash::ZerocashParams p(
        TEST_TREE_DEPTH,
        &keypair
    );
    libzerocash::timer_stop("Param Generation");
    print_mem("after param generation");

    cout << "Successfully created Params.\n" << endl;

    std::string vk_path = "./zerocashTest-verification-key";
    std::string pk_path = "./zerocashTest-proving-key";

    libzerocash::timer_start("Saving Proving Key");

    libzerocash::ZerocashParams::SaveProvingKeyToFile(
        &p.getProvingKey(),
        pk_path
    );

    libzerocash::timer_stop("Saving Proving Key");

    libzerocash::timer_start("Saving Verification Key");

    libzerocash::ZerocashParams::SaveVerificationKeyToFile(
        &p.getVerificationKey(),
        vk_path
    );

    libzerocash::timer_stop("Saving Verification Key");

    libzerocash::timer_start("Loading Proving Key");
    auto pk_loaded = libzerocash::ZerocashParams::LoadProvingKeyFromFile(pk_path, TEST_TREE_DEPTH);
    libzerocash::timer_stop("Loading Proving Key");

    libzerocash::timer_start("Loading Verification Key");
    auto vk_loaded = libzerocash::ZerocashParams::LoadVerificationKeyFromFile(vk_path, TEST_TREE_DEPTH);
    libzerocash::timer_stop("Loading Verification Key");

    cout << "Comparing Proving and Verification key.\n" << endl;

    if ( !( p.getProvingKey() == pk_loaded && p.getVerificationKey() == vk_loaded) ) {
        BOOST_ERROR("Proving and verification key are not equal.");
    }

    vector<libzerocash::Coin> coins;
    vector<libzerocash::Address> addrs;

    cout << "Creating Addresses and Coins...\n" << endl;
    for(size_t i = 0; i < 5; i++) {
        addrs.push_back(libzerocash::Address::CreateNewRandomAddress());
        coins.push_back(libzerocash::Coin(addrs.at(i).getPublicAddress(), i));
    }
    cout << "Successfully created address and coins.\n" << endl;

    cout << "Creating a Mint Transaction...\n" << endl;
    libzerocash::MintTransaction minttx(coins.at(0));
    cout << "Successfully created a Mint Transaction.\n" << endl;

    vector<std::vector<bool>> coinValues(5);
    vector<bool> temp_comVal(ZC_CM_SIZE * 8);
    for(size_t i = 0; i < coinValues.size(); i++) {
        libzerocash::convertBytesVectorToVector(coins.at(i).getCoinCommitment().getCommitmentValue(), temp_comVal);
        coinValues.at(i) = temp_comVal;
    }

    cout << "Creating Merkle Tree...\n" << endl;
    libzerocash::IncrementalMerkleTree merkleTree(coinValues, TEST_TREE_DEPTH);
    cout << "Successfully created Merkle Tree.\n" << endl;

    std::vector<bool> index;

    cout << "Creating Witness 1...\n" << endl;
    merkle_authentication_path witness_1(TEST_TREE_DEPTH);
    libzerocash::convertIntToVector(1, index);
    merkleTree.getWitness(index, witness_1);
    cout << "Successfully created Witness 1.\n" << endl;

    cout << "Creating Witness 2...\n" << endl;
    merkle_authentication_path witness_2(TEST_TREE_DEPTH);
    libzerocash::convertIntToVector(3, index);
    merkleTree.getWitness(index, witness_2);
    cout << "Successfully created Witness 2.\n" << endl;

    cout << "Creating coins to spend...\n" << endl;
    libzerocash::Address newAddress3 = libzerocash::Address::CreateNewRandomAddress();
    libzerocash::PublicAddress pubAddress3 = newAddress3.getPublicAddress();

    libzerocash::Address newAddress4 = libzerocash::Address::CreateNewRandomAddress();
    libzerocash::PublicAddress pubAddress4 = newAddress4.getPublicAddress();

    libzerocash::Coin c_1_new(pubAddress3, 2);
    libzerocash::Coin c_2_new(pubAddress4, 2);
    cout << "Successfully created coins to spend.\n" << endl;

    vector<unsigned char> rt(ZC_ROOT_SIZE);
    merkleTree.getRootValue(rt);

    // XXX: debugging
    std::cout << "Root: " << rt.size() << endl;
    std::cout << "wit1: " << witness_1.size() << endl;
    std::cout << "wit2: " << witness_1.size() << endl;

    vector<unsigned char> as(ZC_SIG_PK_SIZE, 'a');

    cout << "Creating a pour transaction...\n" << endl;
    libzerocash::PourTransaction pourtx(1, p,
    		rt,
    		coins.at(1), coins.at(3),
    		addrs.at(1), addrs.at(3),
                1, 3,
    		witness_1, witness_2,
    		pubAddress3, pubAddress4,
    		0,
            0,
    		as,
    		c_1_new, c_2_new);
    cout << "Successfully created a pour transaction.\n" << endl;

	std::vector<unsigned char> pubkeyHash(ZC_SIG_PK_SIZE, 'a');

    cout << "Verifying a pour transaction...\n" << endl;
    bool pourtx_res = pourtx.verify(p, pubkeyHash, rt);

    BOOST_CHECK(pourtx_res);
}

BOOST_AUTO_TEST_CASE( PourInputOutputTest ) {
    // dummy input
    {
        libzerocash::PourInput input(TEST_TREE_DEPTH);

        BOOST_CHECK(input.old_coin.getValue() == 0);
        BOOST_CHECK(input.old_address.getPublicAddress() == input.old_coin.getPublicAddress());
    }

    // dummy output
    {
        libzerocash::PourOutput output(0);

        BOOST_CHECK(output.new_coin.getValue() == 0);
        BOOST_CHECK(output.to_address == output.new_coin.getPublicAddress());
    }
}

// testing with general situational setup
bool test_pour(libzerocash::ZerocashParams& p,
          uint64_t vpub_in,
          uint64_t vpub_out,
          std::vector<uint64_t> inputs, // values of the inputs (max 2)
          std::vector<uint64_t> outputs) // values of the outputs (max 2)
{
    using pour_input_state = std::tuple<libzerocash::Address, libzerocash::Coin, std::vector<bool>>;

    // Construct incremental merkle tree
    libzerocash::IncrementalMerkleTree merkleTree(TEST_TREE_DEPTH);

    // Dummy sig_pk
    vector<unsigned char> as(ZC_SIG_PK_SIZE, 'a');

    vector<libzerocash::PourInput> pour_inputs;
    vector<libzerocash::PourOutput> pour_outputs;

    vector<pour_input_state> input_state;

    for(std::vector<uint64_t>::iterator it = inputs.begin(); it != inputs.end(); ++it) {
        libzerocash::Address addr = libzerocash::Address::CreateNewRandomAddress();
        libzerocash::Coin coin(addr.getPublicAddress(), *it);

        // commitment from coin
        std::vector<bool> commitment(ZC_CM_SIZE * 8);
        libzerocash::convertBytesVectorToVector(coin.getCoinCommitment().getCommitmentValue(), commitment);

        // insert commitment into the merkle tree
        std::vector<bool> index;
        merkleTree.insertElement(commitment, index);

        // store the state temporarily
        input_state.push_back(std::make_tuple(addr, coin, index));
    }

    // compute the merkle root we will be working with
    vector<unsigned char> rt(ZC_ROOT_SIZE);
    {
        vector<bool> root_bv(ZC_ROOT_SIZE * 8);
        merkleTree.getRootValue(root_bv);
        libzerocash::convertVectorToBytesVector(root_bv, rt);
    }

    // get witnesses for all the input coins and construct the pours
    for(vector<pour_input_state>::iterator it = input_state.begin(); it != input_state.end(); ++it) {
        merkle_authentication_path path(TEST_TREE_DEPTH);

        auto index = std::get<2>(*it);
        merkleTree.getWitness(index, path);

        pour_inputs.push_back(libzerocash::PourInput(std::get<1>(*it), std::get<0>(*it), libzerocash::convertVectorToInt(index), path));
    }

    // construct dummy outputs with the given values
    for(vector<uint64_t>::iterator it = outputs.begin(); it != outputs.end(); ++it) {
        pour_outputs.push_back(libzerocash::PourOutput(*it));
    }

    libzerocash::PourTransaction pourtx(p, as, rt, pour_inputs, pour_outputs, vpub_in, vpub_out);

    BOOST_CHECK(pourtx.verify(p, as, rt));
}

BOOST_AUTO_TEST_CASE( CoinTest ) {
    cout << "\nCOIN TEST\n" << endl;

    libzerocash::Address newAddress = libzerocash::Address::CreateNewRandomAddress();
    libzerocash::PublicAddress pubAddress = newAddress.getPublicAddress();

    libzerocash::Coin coin(pubAddress, 0);

    cout << "Coin Transaction size: " << coin.getCoinSize() << "B" << endl << endl;

    cout << "Successfully created a coin.\n" << endl;

    ///////////////////////////////////////////////////////////////////////////

    libzerocash::timer_start("Coin");
    libzerocash::Coin coin2(pubAddress, 0);
    libzerocash::timer_stop("Coin");

    cout << "Successfully created a coin.\n" << endl;
}

BOOST_AUTO_TEST_CASE( MintTxTest ) {
    cout << "\nMINT TRANSACTION TEST\n" << endl;

    libzerocash::Address newAddress = libzerocash::Address::CreateNewRandomAddress();
    libzerocash::PublicAddress pubAddress = newAddress.getPublicAddress();

    vector<unsigned char> value(ZC_V_SIZE, 0);

    libzerocash::timer_start("Coin");
    const libzerocash::Coin coin(pubAddress, 0);
    libzerocash::timer_stop("Coin");

    libzerocash::timer_start("Mint Transaction");
    libzerocash::MintTransaction minttx(coin);
    libzerocash::timer_stop("Mint Transaction");

    cout << "Mint Transaction size: " << minttx.getMintTxSize() << "B" << endl << endl;

    cout << "Successfully created a mint transaction.\n" << endl;

    libzerocash::timer_start("Mint Transaction Verify");
    bool minttx_res = minttx.verify();
    libzerocash::timer_stop("Mint Transaction Verify");

    BOOST_CHECK(minttx_res);
}

BOOST_AUTO_TEST_CASE ( ReceivePourTxTest ) {
    cout << "\nCREATE AND RECEIVE POUR TRANSACTION TEST\n" << endl;

    cout << "Creating Params...\n" << endl;

    libzerocash::timer_start("Param Generation");
    auto keypair = libzerocash::ZerocashParams::GenerateNewKeyPair(TEST_TREE_DEPTH);
    libzerocash::ZerocashParams p(
        TEST_TREE_DEPTH,
        &keypair
    );
    libzerocash::timer_stop("Param Generation");
    print_mem("after param generation");

    cout << "Successfully created Params.\n" << endl;

    vector<libzerocash::Coin> coins;
    vector<libzerocash::Address> addrs;

    for(size_t i = 0; i < 5; i++) {
        addrs.push_back(libzerocash::Address::CreateNewRandomAddress());
        coins.push_back(libzerocash::Coin(addrs.at(i).getPublicAddress(), i));
    }

    cout << "Successfully created coins.\n" << endl;

    vector<std::vector<bool>> coinValues(5);

    vector<bool> temp_comVal(ZC_CM_SIZE * 8);
    for(size_t i = 0; i < coinValues.size(); i++) {
        libzerocash::convertBytesVectorToVector(coins.at(i).getCoinCommitment().getCommitmentValue(), temp_comVal);
        coinValues.at(i) = temp_comVal;
        libzerocash::printVectorAsHex("Coin => ", coinValues.at(i));
    }

    cout << "Creating Merkle Tree...\n" << endl;

    libzerocash::timer_start("Merkle Tree");
    libzerocash::IncrementalMerkleTree merkleTree(coinValues, TEST_TREE_DEPTH);
    libzerocash::timer_stop("Merkle Tree");

    cout << "Successfully created Merkle Tree.\n" << endl;

    merkle_authentication_path witness_1(TEST_TREE_DEPTH);

    libzerocash::timer_start("Witness");
    std::vector<bool> index;
    libzerocash::convertIntToVector(1, index);
    if (merkleTree.getWitness(index, witness_1) == false) {
        BOOST_ERROR("Could not get witness");
	}
    libzerocash::timer_stop("Witness");

    cout << "Witness 1: " << endl;
    for(size_t i = 0; i < witness_1.size(); i++) {
        libzerocash::printVectorAsHex(witness_1.at(i));
    }
    cout << "\n" << endl;

    merkle_authentication_path witness_2(TEST_TREE_DEPTH);
    libzerocash::convertIntToVector(3, index);
    if (merkleTree.getWitness(index, witness_2) == false) {
		cout << "Could not get witness" << endl;
	}

    cout << "Witness 2: " << endl;
    for(size_t i = 0; i < witness_2.size(); i++) {
        libzerocash::printVectorAsHex(witness_2.at(i));
    }
    cout << "\n" << endl;

    libzerocash::Address newAddress3 = libzerocash::Address::CreateNewRandomAddress();
    libzerocash::PublicAddress pubAddress3 = newAddress3.getPublicAddress();

    libzerocash::Address newAddress4 = libzerocash::Address::CreateNewRandomAddress();
    libzerocash::PublicAddress pubAddress4 = newAddress4.getPublicAddress();

    libzerocash::Coin c_1_new(pubAddress3, 2);
    libzerocash::Coin c_2_new(pubAddress4, 2);

    vector<bool> root_bv(ZC_ROOT_SIZE * 8);
    merkleTree.getRootValue(root_bv);
    vector<unsigned char> rt(ZC_ROOT_SIZE);
    libzerocash::convertVectorToBytesVector(root_bv, rt);

    vector<unsigned char> ones(ZC_V_SIZE, 1);
    vector<unsigned char> twos(ZC_V_SIZE, 2);
    vector<unsigned char> as(ZC_SIG_PK_SIZE, 'a');

    cout << "Creating a pour transaction...\n" << endl;

    libzerocash::timer_start("Pour Transaction");
    libzerocash::PourTransaction pourtx(1, p, rt, coins.at(1), coins.at(3), addrs.at(1), addrs.at(3), 1, 3, witness_1, witness_2, pubAddress3, pubAddress4, 0, 0, as, c_1_new, c_2_new);
    libzerocash::timer_stop("Pour Transaction");
    print_mem("after pour transaction");

    cout << "Pour Transaction size: " << pourtx.getPourTxSize() << "B" << endl << endl;

    cout << "Successfully created a pour transaction.\n" << endl;

    std::vector<unsigned char> pubkeyHash(ZC_SIG_PK_SIZE, 'a');

    libzerocash::timer_start("Pour Transaction Verify");
    bool pourtx_res = pourtx.verify(p, pubkeyHash, rt);
    libzerocash::timer_stop("Pour Transaction Verify");

    BOOST_CHECK(pourtx_res);

    cout << "Receiving a pour transaction...\n" << endl;
    libzerocash::timer_start("Receiving coins");
    libzerocash::Coin recvCoin1(pourtx.getCiphertext1(), newAddress3);
    libzerocash::Coin recvCoin2(pourtx.getCiphertext2(), newAddress4);
    libzerocash::timer_stop("Receiving coins");
    cout << "coin1 value: " << recvCoin1.getValue()<< endl;
    cout << "coin2 value: " << recvCoin2.getValue()<< endl; 
    cout << "Successfully received a pour transaction.\n" << endl;
}