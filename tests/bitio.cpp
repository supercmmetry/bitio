#include <gtest/gtest.h>
#include <bitio/bitio.h>
#include <chrono>

class BitioTest : testing::Test {
};

TEST(BitioTest, read_test_1) {
    // Create sample file
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    uint8_t *raw = new uint8_t[1048576];

    for (int i = 0; i < 1048576; i++) {
        raw[i] = i % 256;
    }

    fwrite(raw, 1, 1048576, file);
    fclose(file);


    file = fopen("bitio_test.dat", "rb");

    auto stream = new bitio::stream(file);

    int count = 10;
    for (int i = 0; i < count; i++) {
        stream->read(0x4);
        stream->read(0x1);
        stream->read(0xB);
    }

    ASSERT_EQ(stream->read(0x8), 20);
    

    delete stream;
}

TEST(BitioTest, read_test_2) {
    // Create sample file
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    uint8_t *raw = new uint8_t[100];

    for (int i = 0; i < 100; i++) {
        raw[i] = 0xff;
    }

    fwrite(raw, 1, 100, file);
    fclose(file);

    file = fopen("bitio_test.dat", "rb");

    auto stream = new bitio::stream(file);

    ASSERT_EQ(stream->read(0x8), 0xff);
    ASSERT_EQ(stream->read(0x1), 1);
    ASSERT_EQ(stream->read(0x2), 3);
    ASSERT_EQ(stream->read(0x3), 7);
    ASSERT_EQ(stream->read(0x4), 15);
    ASSERT_EQ(stream->read(0x5), 31);
    ASSERT_EQ(stream->read(0x6), 63);
    ASSERT_EQ(stream->read(0x7), 127);
    ASSERT_EQ(stream->read(10), 1023);
    ASSERT_EQ(stream->read(20), 1048575);

    delete stream;
}

TEST(BitioTest, read_test_3) {
    // Create sample file
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    uint8_t *raw = new uint8_t[5];

    for (int i = 0; i < 5; i++) {
        raw[i] = i;
    }

    fwrite(raw, 1, 5, file);
    fclose(file);


    file = fopen("bitio_test.dat", "rb");

    auto stream = new bitio::stream(file, 2);

    int count = 2;
    for (int i = 0; i < count; i++) {
        stream->read(0x10);
    }

    ASSERT_EQ(stream->read(0x8), (count << 1) % 256);

    

    delete stream;
}

TEST(BitioTest, read_test_4) {
    // Create sample file
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    uint8_t *raw = new uint8_t[1024];

    for (int i = 0; i < 1024; i++) {
        raw[i] = i % 256;
    }

    fwrite(raw, 1, 1024, file);
    fclose(file);


    file = fopen("bitio_test.dat", "rb");

    auto stream = new bitio::stream(file, 1);

    for (int i = 0; i < 32; i++) {
        stream->read(64);
    }

    ASSERT_EQ(stream->read(8), 0);
}

TEST(BitioTest, write_test_1) {
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");

    auto stream = new bitio::stream(file, 1024);
    stream->write(4, 8);
    stream->write(10, 5);
    stream->write(0xffff, 16);
    stream->write(0xffff, 64);
    stream->flush();

    delete stream;

    file = fopen("bitio_test.dat", "rb");
    stream = new bitio::stream(file, 2);

    ASSERT_EQ(stream->read(8), 4);
    ASSERT_EQ(stream->read(5), 10);
    ASSERT_EQ(stream->read(16), 0xffff);
    ASSERT_EQ(stream->read(64), 0xffff);
}

TEST(BitioTest, write_test_2) {
    FILE *file = fopen("bitio_test.dat", "wb");

    auto stream = new bitio::stream(file, 1);
    for (int i = 0; i < 1024; i++) {
        stream->write(i % 256, 12 + (i % 32));
    }

    stream->flush();
    

    file = fopen("bitio_test.dat", "rb");
    stream = new bitio::stream(file, 1);

    for (int i = 0; i < 1024; i++) {
        ASSERT_EQ(stream->read(12 + (i % 32)), i % 256);
    }
}

TEST(BitioTest, write_test_3) {
    FILE *file = fopen("bitio_test.dat", "wb");

    auto stream = new bitio::stream(file, 13);
    for (int i = 0; i < 128; i++) {
        stream->write(i % 256, 64);
    }

    stream->flush();

    delete stream;

    file = fopen("bitio_test.dat", "rb");
    stream = new bitio::stream(file, 19);

    for (int i = 0; i < 128; i++) {
        ASSERT_EQ(stream->read(64), i % 256);
    }
}

TEST(BitioTest, seek_test_1) {
    remove("bitio_test.dat");
    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, 1);
    stream->write(0xfeff, 16);
    stream->seek(-0x8);
    stream->flush();
    ASSERT_EQ(stream->read(0x8), 0xff);
}

TEST(BitioTest, seek_test_2) {
    remove("bitio_test.dat");
    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, 1);
    stream->write(0xffff, 16);
    stream->seek(-0x1);
    stream->seek(-0x7);
    stream->seek(0x8);
    stream->seek(-0x8);

    ASSERT_EQ(stream->read(0x8), 0xff);
}

TEST(BitioTest, seek_rw_test_1) {
    remove("bitio_test.dat");

    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, 100);
    stream->write(0xfff3, 16);
    stream->seek(-0x8);
    stream->write(0xfe, 0x8);
    stream->seek(-0x8);

    ASSERT_EQ(stream->read(0x8), 0xfe);
}

TEST(BitioTest, seek_rw_test_2) {
    remove("bitio_test.dat");
    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, 100);

    stream->write(0xff, 12);
    stream->seek(-1);
    stream->seek(-11);
    ASSERT_EQ(stream->read(12), 0xff);

    stream->write(0xff, 10);
    stream->seek(-10);
    stream->write(0xfe, 10);
    stream->seek(-10);
    ASSERT_EQ(stream->read(10), 0xfe);
}

TEST(BitioTest, seek_rw_test_3) {
    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, 1);

    stream->write(0xff, 12);
    stream->seek(-1);
    stream->seek(-11);
    ASSERT_EQ(stream->read(12), 0xff);


    stream->write(0xff, 10);
    stream->seek(-10);
    stream->write(0xfe, 10);
    stream->seek(-10);
    ASSERT_EQ(stream->read(10), 0xfe);
}

TEST(BitioTest, seek_rw_test_4) {
    remove("bitio_test.dat");
    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, 2);

    stream->write(0xff, 12);
    stream->seek(-1);
    stream->seek(-11);
    ASSERT_EQ(stream->read(12), 0xff);

    stream->write(0xff, 10);
    stream->seek(-10);
    stream->write(0xfe, 10);
    stream->seek(-10);
    ASSERT_EQ(stream->read(10), 0xfe);
}

TEST(BitioTest, seek_rw_test_5) {
    remove("bitio_test.dat");

    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, 2);

    stream->write(1234, 11);
    stream->seek(-3);
    stream->seek(-5);
    stream->read(1);
    stream->seek(-4);

    ASSERT_EQ(stream->read(11), 1234);

    stream->flush();

    delete stream;

    file = fopen("bitio_test.dat", "rb+");
    stream = new bitio::stream(file, 1);

    ASSERT_EQ(stream->read(11), 1234);
}

TEST(BitioTest, seek_rw_test_6) {
    remove("bitio_test.dat");

    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, 1);

    stream->write(1234, 11);
    stream->seek(-3);
    stream->seek(-5);
    stream->write(0xff, 0x8);
    stream->seek(-1);
    stream->write(0x0, 0x1);
    stream->seek(-8);

    ASSERT_EQ(stream->read(8), 0xfe);
}

TEST(BitioTest, seek_rw_test_7) {
    remove("bitio_test.dat");

    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, 10);

    stream->write(1234, 11);
    stream->seek(-3);
    stream->seek(-5);
    stream->write(0xff, 0x8);
    stream->seek(-1);
    stream->write(0x0, 0x1);
    stream->seek(-8);

    ASSERT_EQ(stream->read(8), 0xfe);
}

TEST(BitioTest, seek_rw_test_8) {
    remove("bitio_test.dat");

    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, 1);

    for (int i = 0; i < 2; i++) {
        stream->write(i, 8);
        stream->seek(-1);
    }

    stream->seek(-14);

    ASSERT_EQ(stream->read(15), 1);
}

TEST(BitioTest, rw_test_1) {
    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, 1);

    stream->write(0xff, 8);
    stream->write(0xfe, 8);
    stream->write(0xfc, 8);
    stream->flush();
    

    file = fopen("bitio_test.dat", "rb+");
    stream = new bitio::stream(file, true);

    stream->write(0x01, 8);
    ASSERT_EQ(stream->read(8), 0xfe);
    stream->write(0x02, 8);

    stream->flush();
    
}

TEST(BitioTest, rw_test_2) {
    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, 3);

    stream->write(0xfffefdfc, 32);
    stream->seek(-32);
    ASSERT_EQ(stream->read(0x8), 0xff);
    stream->seek(-8);
    ASSERT_EQ(stream->read(0x8), 0xff);
    ASSERT_EQ(stream->read(0x8), 0xfe);
    stream->seek(-4);
    ASSERT_EQ(stream->read(0x4), 0xe);
    stream->seek(0x8);
    ASSERT_EQ(stream->read(0x8), 0xfc);

    stream->flush();
    
}

TEST(BitioTest, rw_test_3) {
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    auto stream = new bitio::stream(file, 3);

    stream->write(29, 5);
    stream->seek(-1);
    ASSERT_EQ(stream->read(1), 0x1);

    stream->write(0xfb, 11);
    stream->write(0xff, 8);
    stream->write(0xff33ff, 60);

    stream->flush();
    
    delete stream;

    file = fopen("bitio_test.dat", "rb+");
    stream = new bitio::stream(file, 2);

    ASSERT_EQ(stream->read(5), 29);
    ASSERT_EQ(stream->read(11), 0xfb);
    ASSERT_EQ(stream->read(8), 0xff);
    ASSERT_EQ(stream->read(60), 0xff33ff);

    ASSERT_EQ(stream->size(), 11);
}

TEST(BitioTest, rw_test_4) {
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    auto stream = new bitio::stream(file, 100);

    stream->write(29, 5);
    stream->seek(-1);
    ASSERT_EQ(stream->read(1), 0x1);

    stream->write(0xfb, 11);
    stream->write(0xff, 8);
    stream->write(0xff33ff, 25);

    stream->flush();

    delete stream;

    file = fopen("bitio_test.dat", "rb+");
    stream = new bitio::stream(file, 200);

    ASSERT_EQ(stream->read(5), 29);
    ASSERT_EQ(stream->read(11), 0xfb);
    ASSERT_EQ(stream->read(8), 0xff);
    ASSERT_EQ(stream->read(25), 0xff33ff);

    ASSERT_EQ(stream->size(), 7);
}

TEST(BitioTest, rw_test_5) {
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    auto stream = new bitio::stream(file, 2);

    stream->write(29, 5);
    stream->seek(-1);
    ASSERT_EQ(stream->read(1), 0x1);

    stream->write(0xfb, 11);
    stream->write(0xff, 8);
    stream->write(0xff33ff, 25);
    stream->seek(-44);
    stream->seek(19);

    ASSERT_EQ(stream->read(25), 0xff33ff);

    stream->flush();
    

    file = fopen("bitio_test.dat", "rb+");
    stream = new bitio::stream(file, 1);

    ASSERT_EQ(stream->read(5), 29);
    ASSERT_EQ(stream->read(11), 0xfb);
    ASSERT_EQ(stream->read(8), 0xff);
    ASSERT_EQ(stream->read(25), 0xff33ff);

    ASSERT_EQ(stream->size(), 7);
}

TEST(BitioTest, rw_test_6) {
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    auto stream = new bitio::stream(file, 2);

    stream->write(29, 5);
    stream->seek(-1);
    ASSERT_EQ(stream->read(1), 0x1);

    stream->write(0x7fb, 11);
    stream->write(0xff, 8);
    stream->write(0xff33ff, 25);
    stream->seek(-44);
    stream->seek(19);

    ASSERT_EQ(stream->read(25), 0xff33ff);

    stream->flush();
    

    file = fopen("bitio_test.dat", "rb+");
    stream = new bitio::stream(file, 2);

    ASSERT_EQ(stream->size(), 7);

    stream->write(29, 5);
    ASSERT_EQ(stream->read(11), 0x7fb);
    stream->write(0xfe, 0x8);
    ASSERT_EQ(stream->read(25), 0xff33ff);
}

TEST(BitioTest, rw_test_7) {
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    auto stream = new bitio::stream(file, 2);

    stream->write(29, 5);
    stream->seek(-1);
    ASSERT_EQ(stream->read(1), 0x1);

    stream->write(0x7fb, 11);
    stream->write(0xff, 8);
    stream->write(0xff33ff, 25);
    stream->seek(-44);
    stream->seek(19);

    ASSERT_EQ(stream->read(25), 0xff33ff);

    stream->flush();
    

    file = fopen("bitio_test.dat", "rb+");
    stream = new bitio::stream(file, 1);

    ASSERT_EQ(stream->size(), 7);

    stream->write(29, 5);
    ASSERT_EQ(stream->read(11), 0x7fb);
    stream->write(0xfe, 0x8);
    ASSERT_EQ(stream->read(25), 0xff33ff);
}

TEST(BitioTest, rw_test_8) {
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    auto stream = new bitio::stream(file, 3);

    stream->write(29, 5);
    stream->seek(-1);
    ASSERT_EQ(stream->read(1), 0x1);

    stream->write(0x7fb, 11);
    stream->write(0xff, 8);
    stream->write(0xff33ff, 25);
    stream->write(0x3434, 39);
    stream->write(0x4343, 40);

    stream->flush();

    delete stream;

    file = fopen("bitio_test.dat", "rb+");
    stream = new bitio::stream(file, 2);

    stream->write(29, 5);
    ASSERT_EQ(stream->read(11), 0x7fb);
    stream->write(0xfe, 0x8);
    ASSERT_EQ(stream->read(25), 0xff33ff);
    stream->write(0x4343, 39);
    ASSERT_EQ(stream->read(40), 0x4343);
    stream->seek(-40);
    stream->write(0x3434, 40);
    stream->seek(-40);
    ASSERT_EQ(stream->read(40), 0x3434);
}

TEST(BitioTest, rw_test_9) {
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    auto stream = new bitio::stream(file, 3);

    stream->write(29, 8);
    stream->seek(-1);
    ASSERT_EQ(stream->read(1), 0x1);

    stream->write(0x7f, 8);
    stream->write(0xff, 8);
    stream->write(0xff33ff, 24);
    stream->write(0x3434, 16);
    stream->write(0x4343, 16);

    stream->flush();

    delete stream;

    file = fopen("bitio_test.dat", "rb+");
    stream = new bitio::stream(file, 3);

    stream->write(29, 8);
    ASSERT_EQ(stream->read(8), 0x7f);
    stream->write(0xfe, 0x8);
    ASSERT_EQ(stream->read(24), 0xff33ff);
    stream->write(0x0, 1);
    stream->write(0x5, 3);
    stream->write(0x3, 4);
    stream->write(0x43, 8);
    stream->seek(-16);
    stream->seek(16);
    stream->seek(-16);
    ASSERT_EQ(stream->read(16), 0x5343);
    ASSERT_EQ(stream->read(4), 0x4);
    ASSERT_EQ(stream->read(12), 0x343);
    stream->seek(-16);
    stream->write(0x3434, 16);
    stream->seek(-16);
    ASSERT_EQ(stream->read(16), 0x3434);

    stream->flush();
    
}

TEST(BitioTest, seek_to_1) {
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    auto stream = new bitio::stream(file, 1048576);

    for (int i = 0; i < 1048576; i++) {
        stream->write(0x7f, 8);
        stream->write(0xff, 8);
        stream->write(0x3434, 16);
        stream->write(0x4343, 16);
    }

    stream->flush();
    


    file = fopen("bitio_test.dat", "rb+");
    stream = new bitio::stream(file, 1048576);

    stream->seek_to(48 * 1048575 + 16);

    ASSERT_EQ(stream->read(16), 0x3434);
}

TEST(BitioTest, seek_to_2) {
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    auto stream = new bitio::stream(file, 3);

    stream->write(0x7f, 8);
    stream->write(0xff, 8);
    stream->write(0x3434, 16);
    stream->write(0x4343, 16);

    stream->flush();
    

    file = fopen("bitio_test.dat", "rb+");
    stream = new bitio::stream(file, 1);

    stream->seek_to(16);
    ASSERT_EQ(stream->read(16), 0x3434);

    stream->seek_to(0);
    stream->seek(16);
    stream->write(0x4444, 16);
    stream->seek_to(15);
    stream->seek(-7);
    stream->seek(-3);
    stream->seek(10);
    stream->seek(1);
    ASSERT_EQ(stream->read(16), 0x4444);

    stream->flush();
    
}

TEST(BitioTest, seek_to_write_read_1) {
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    auto stream = new bitio::stream(file, 1024);

    for (int i = 0; i < 1024; i++) {
        stream->write(0xf0f1f2f3f4f5f6f7, 0x40);
    }

    stream->seek_to(0);
    stream->write(0x13, 0x8);
    stream->read(0x40);

    stream->seek_to(0);
    ASSERT_EQ(stream->read(0x8), 0x13);
}

TEST(BitioTest, seek_to_write_read_2) {
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    auto stream = new bitio::stream(file, 1);

    for (int i = 0; i < 1024; i++) {
        stream->write(0xf0f1f2f3f4f5f6f7, 0x40);
    }

    stream->seek_to(0);
    stream->write(0x13, 0x8);
    stream->read(0x40);

    stream->seek_to(0);
    ASSERT_EQ(stream->read(0x8), 0x13);
}

TEST(BitioTest, seek_to_write_read_3) {
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    auto stream = new bitio::stream(file, 0x40);

    for (int i = 0; i < 1024; i++) {
        stream->write(0xf0f1f2f3f4f5f6f7, 0x40);
    }

    stream->seek_to(0);
    stream->write(0x13, 0x8);
    stream->read(0x40);

    stream->seek_to(0);
    ASSERT_EQ(stream->read(0x8), 0x13);
}

TEST(BitioTest, seek_to_write_read_4) {
    remove("bitio_test.dat");

    auto *tmp = fopen("bitio_test.dat", "a");
    fclose(tmp);

    FILE *file = fopen("bitio_test.dat", "rb+");
    auto stream = new bitio::stream(file, 0x40);

    for (int i = 0; i < 1024; i++) {
        stream->write(0xf0f1f2f3f4f5f6f7, 0x40);
    }

    stream->seek_to(0);
    stream->write(0x13, 0x8);
    stream->seek(0x8);
    stream->read(0x40);
    stream->read(0x32);

    stream->seek_to(0);
    ASSERT_EQ(stream->read(0x8), 0x13);
}

TEST(BitioTest, raw_buffer_1) {
    auto raw = new uint8_t[20];
    auto stream = bitio::stream(raw, 20);

    for (int i = 0; i < 20; i++) {
        stream.write(1, 0x8);
    }

    stream.seek_to(0);

    for (int i = 0; i < 19; i++) {
        stream.write(1, 0x8);
    }

    stream.write(1, 0x1);

    stream.seek_to(0);
    stream.seek(20 * 8);
    stream.seek(-20 * 8);

    for (int i = 0; i < 19; i++) {
        ASSERT_EQ(stream.read(0x8), 1);
    }

    ASSERT_EQ(stream.read(0x8), 129);

    delete[] raw;
}