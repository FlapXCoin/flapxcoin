// Copyright (c) 2009-2023 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "txdb.h"
#include "main.h"
#include "uint256.h"


static const int nCheckpointSpan = 10;

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    //
    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    //
    // FlapX 2.9.1.0: every 25,000 blocks from genesis to 4,425,000, read from a
    // fully synced node and checked against all earlier checkpoints at the same
    // heights (55 matched). The old odd heights (5, 1284933, 1285512) were
    // against forks that have long been resolved.
    //
    // The genesis hashes are written out rather than read from
    // Params().HashGenesisBlock(): these maps are built during static
    // initialisation, before chainparams.cpp has constructed its params, so
    // Params() returned an all-zero hash here and a node with an empty data
    // directory could not find any checkpoint and failed to start.
static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (      0, uint256("0xf30d6d37c276519974c14eae52dac5538b2bdc358f2e4a3b48a14b201eb48fed"))
        (  25000, uint256("0x3afbd90e6af40fafe1448a7411d2d539aacdceea491ad0283d97782f125788b2"))
        (  50000, uint256("0xc37f17964a5dd2f1e0620e4626283bfa5eba0890e7e7711f3e02b655440ddfdb"))
        (  75000, uint256("0x7171dcf65fde45be9cb48da9a6a4425982acb39cef67e2a6150cd03be79992bf"))
        ( 100000, uint256("0xbfbe09c7103b672ce64e99b8b02b1eb1a5d7d39ce211331bb9ad3ae6f7d381ed"))
        ( 125000, uint256("0x0c4a145b60b899bcaef36d2c26784b7550dc9f38c9d1ee6866e084e03b3b0372"))
        ( 150000, uint256("0x8f282b87a5ec547753bd54f8d98ddac174ac4fac3c85d852071a16f9e088a20b"))
        ( 175000, uint256("0x881b5e81a5367994a9604d48b5b3df8f1038f0f75b78e47ee88a867559e16ca7"))
        ( 200000, uint256("0xe66c728d7ef8986afe6009a292f76161658882ea7adc3eb6f9e59db4e7a76849"))
        ( 225000, uint256("0x76bbbb56a9aff06d5e21db2ee22984b78e91cdb2da41c3300f299bf549a6b6e8"))
        ( 250000, uint256("0x62dab06a1296eee06a51d2944f9ce1465e71ab15f3dd291505113341813f0d05"))
        ( 275000, uint256("0xa660fc539487844e36a5811bd63ca28eb71305b9ea9c5efabcd34e333d2cd6ce"))
        ( 300000, uint256("0x6786bc4f06982eeaa218b526aa181cf8188e8eb33b8418de8996001a5dca982a"))
        ( 325000, uint256("0x67269ff31c308d77802504fa0ba5c99126d34769a9f6461eae4644627813e5de"))
        ( 350000, uint256("0x3cb61c2848edf501334f20a2f87bb9d1d2b55e52c44c46ca05ce9c9a82b60a05"))
        ( 375000, uint256("0x0d7a517017d199b3f296de558905611df9b3f21e6ec38ec9ed73a084f1dabb20"))
        ( 400000, uint256("0x1d7befbec411d8134306b621aa6758709f9a8209313a447869538f5fe4048b65"))
        ( 425000, uint256("0x4d28bae2121624890cd2c35c239892149ee39a0285c362bcc003fa9742c8b70a"))
        ( 450000, uint256("0x2d23e1061d7d516bb6c46e2da85915dd0f22728e1602e08243d7b4cd575550b9"))
        ( 475000, uint256("0xd46ba1322d8001f8fc926af40caea3b92ca56ba60a69a08ed66b8b68c19c56de"))
        ( 500000, uint256("0xdb03b4adda738d910b07def47870ce87e9b792cf024468b73beda8f07bbf6c39"))
        ( 525000, uint256("0x00f74ae5060e1ff3360d96e52cd7d6df8e62317f4e1d6c47d5f2f3853ce59926"))
        ( 550000, uint256("0xc99b192e66a0d5db2a67a3b1b0a607f689ae0178037c34530c2ddc12cbb3d49e"))
        ( 575000, uint256("0x044ad75baab179ffde0ae5221137f1848cdc88c5096f1cce7f8e2edc9b8abbbf"))
        ( 600000, uint256("0x62458e42995e5c7b7f01443c94161921b2e55d874270c488bcb3bd82467ae06a"))
        ( 625000, uint256("0x7de0b393331c7e7da493d40107b3733b22c6b2c49eeabfb769d73d80524471b6"))
        ( 650000, uint256("0xcac0df1cb10a311d8b9bd53e803b661d7d44ac118a9f5ed5fbe97546b4572524"))
        ( 675000, uint256("0xd6de6441bca1df6f7f747b1d0bb0018df15750ab82f09ecca5fb47a674a59156"))
        ( 700000, uint256("0xed6761a21f69e5b3bd51cbdaf81cef02bd9a9c0d3a5aaad1e886831ffd8f9452"))
        ( 725000, uint256("0xd0d08efe541a151a566e90987d8fdda5a9a759f083f65cc4663c801cccaa2ac6"))
        ( 750000, uint256("0x0311b62721b49f695154db60553f709f224b59f4c94dd2befe9abb781e79c44e"))
        ( 775000, uint256("0x12e4c193f6aa10b1694f41a72c179ba5781cc67470f89f0d6e87a77909c219e9"))
        ( 800000, uint256("0xbcf9f4a5a1291a6dc35b8078b2e5bb94f265d877cbcf5a9678429a35579c0ae5"))
        ( 825000, uint256("0xfea99497b0a722d0e2e693c886eb64f7a1421b5eda6cc71017be051967302012"))
        ( 850000, uint256("0x82614434cc3e63839cdc3805c8c90689f5dcf87821e6a39820cd50914d2575b4"))
        ( 875000, uint256("0x13df096c5de4a11949e66bd333022c431f99df77a5cf6fe16c1b62a5c73f0f52"))
        ( 900000, uint256("0xda053bc86b0cda2d26a45be64bec2959b77d37a44d717debafec10c45770faac"))
        ( 925000, uint256("0xbacdc35532be65e08ce5888636c7d2962230f733f8292642bc77eb9077477761"))
        ( 950000, uint256("0xb33dd8c5f481970b0b12fab8920a4b35163dd5e89c4e6671fbd5664d87dee13b"))
        ( 975000, uint256("0xdecf9f223ba788df46853d188e61684eed546b922a811d3fb02892e0e1254ac2"))
        (1000000, uint256("0x55b8b3aa75a004b405e6c70ca924c25d95cb0ed0f95c5b3fdbb48a5708a54693"))
        (1025000, uint256("0xcd2a90830674124400a0899a2c9ddfa97196c05b3974e1449754f71727354cd3"))
        (1050000, uint256("0x33fc9c996b7e62e0c6e9bffd08be7ccc25811ab198398f581e5216767e121d71"))
        (1075000, uint256("0x56a6315863c77b6cfed91976274d2e346c03b568cd723f97258d897dd6a02281"))
        (1100000, uint256("0x43449510f872065f532ed668e561caa3c00557775a6c3c0676e02f1f36fb50d8"))
        (1125000, uint256("0xae5ef2418b2aa51cfa5af7d117927b6d18f5ba4adbe8c60dc2237c0a5ede26fb"))
        (1150000, uint256("0xa266f5a73efbfb65194d241506743ef0f08f7316908b79b35b09fdd35f0452fa"))
        (1175000, uint256("0xe2c11caff5555d329f17d888b4a92a3106b51e094a70a7fbffdf44ffc134b08f"))
        (1200000, uint256("0x31cb8e35ee44835f3dd387c42607c7921afce08c09dabd18d8b83d56df3ed3b9"))
        (1225000, uint256("0x28a95a10bd256f5562bc81897746adf9c7db0bb1f98aece93fdde1e7b2790e5c"))
        (1250000, uint256("0x577ec3afa7bd8011c495b16ca49305847c81c4602150ac77e5475befcc38ca59"))
        (1275000, uint256("0xa04017b12664d9ddde808faeccfae98406a03193546f1ad7311799b7da181d29"))
        (1300000, uint256("0xc18598869ee398ca7d2cb2146c74cff5853c3cc02093500cd79f7eac184efaf2"))
        (1325000, uint256("0xe6e454f922ec57b5281804794ad1dd7677eb20dc9f8dee03bac9222c6e6e7c09"))
        (1350000, uint256("0xe0894e502ca83f77dc7ac2b3b9cb2acbfe4893b411e7b07b9d895916e8e8141f"))
        (1375000, uint256("0xf5519685e4cbe2e80690fb7162e8efa837f95a2df16bd10106ca153992ad943d"))
        (1400000, uint256("0x03e964fec5c36709598b3a731e9249c5d7e10184dd25e860a7748bff53de4f81"))
        (1425000, uint256("0xf7aca597ae34d199687fd95f4d93055c66a69f28a1c6ece0957866b6be594937"))
        (1450000, uint256("0xc23e3165dd41654d417316f3fbc794a9a6b7fd4b16699afb3e3119fbeffe6b3d"))
        (1475000, uint256("0x996b31ffc0e8197bc92919bf6ac8b764675c9af32ef0d8fdf24408842c94d42b"))
        (1500000, uint256("0x06f28a0fe6afd55ab0aa1c0bc7a78f2e1e398f6aafd3d842cd37abf97bc86c1f"))
        (1525000, uint256("0x4360d2d3713d080315e62607d9cd925a05dfebe3c8ace4f131cc269e974de2d8"))
        (1550000, uint256("0x4625eb8e0b6667bf8043e237d316cd6b3eba9778ef1c9d942d04b7960eccabc3"))
        (1575000, uint256("0xfaad4b6037d75626f7969263fa49886c5634c5f7400a0d07847f8d7a05e67582"))
        (1600000, uint256("0xd80365f6170441e838c5db9e746462c6079b220818d14389b03c7da907f7b616"))
        (1625000, uint256("0x146c9c972cf0a206531095cceccb5674a45f6f837c51e5a9396c4f8794dcbbc1"))
        (1650000, uint256("0xcb16cd08ff145b19695d10cdd8cc48be06f06104698f3f97b66d118e4ad271c3"))
        (1675000, uint256("0xbe57882938d5a5ef663e98201d80ae92f559ffbd3442b0f14f8ed2f2da1b58a1"))
        (1700000, uint256("0xe3daf0f87e3c4a77f1768c652e323a3a9dea18091d8793a6bb38adebc1584386"))
        (1725000, uint256("0xbe4951b7dea8afa9478ea97189b19bfd56bcac308ce9ab6160f73de6511736ce"))
        (1750000, uint256("0x5b8ee6b7eab27882ee38cdab4739faf5830c21520aee39a131094bfe4d8306d0"))
        (1775000, uint256("0x7670d3c79ee49cc32f017e6fd44f5d70712384ce60bea3ada884573ee07c29cc"))
        (1800000, uint256("0xb6913150349ab572c4563a74506e2b9e4c757229f51c973a68c6b128e44945f4"))
        (1825000, uint256("0x3be86fcc70a1429ed5edec42cebb440c08347428a69f15770b5084b084c6ec9b"))
        (1850000, uint256("0x73d1fe2d41965c04551b82163cfb89de2c4e2621347c1a4bb5827f17320839a1"))
        (1875000, uint256("0x2a1abd0cb3c03ed872cdba21cf214a128efa7cb4337015017c446878131a9586"))
        (1900000, uint256("0x10a17638f5745ef5f6208af702d81f74674e5fc09157001cda66e3e7714f762b"))
        (1925000, uint256("0x71dea851c44a34b00828e33c37be78bee0219fbf3e8999145b87eeb710d7e99f"))
        (1950000, uint256("0x62cdaf0c82704acb19ce9dbf659bff978a51fc25e8bd540ca4d552e1b9ea73df"))
        (1975000, uint256("0x6dc04fdccebfcdff94ef67d6fa559ebba2e0f47ddbd1c56093a1c14c597c783f"))
        (2000000, uint256("0xf1a60d75a54bd6907eb42dca5e574d3410bbeb89b507768b376a79d408b37ca1"))
        (2025000, uint256("0x553f87517b500367ac51b90e4d7ad923fef024ab83a3c39c9844273e564b3bdf"))
        (2050000, uint256("0x6d03db70ea03b59041838cb4260cbb5df26e672b5a7d504034b0b7f36d867c0f"))
        (2075000, uint256("0x8234aca54baebe200c1170602965be1889626fc88214e25895e60f1d62166b4f"))
        (2100000, uint256("0x89c6ceec919059f9d9c8960ebe3c34adc195c96844d91b84dba1f51958cf5c03"))
        (2125000, uint256("0xa510fe4a08cad8e2cf7837eda0ab861f7e68a50c211a159b7e855f56d5e575f6"))
        (2150000, uint256("0xe52b99220d99d3236614bf40117a34b625667f7abebeb5caba0f16d42b517d82"))
        (2175000, uint256("0x1d981f3d7748536324a023c3f42f1b08c4260057109be338b0650c6d80c7f40b"))
        (2200000, uint256("0xd0b5357c5133bfd4a36cf828dcb3b1c1ec0413fcf6042eed730329b04ff94bed"))
        (2225000, uint256("0x57672970a2be38cf7d9be1c2f4e8ba1394b97f91f797f3917d5321d89bf57165"))
        (2250000, uint256("0x05f3b7765d2eed39fba2c31e90bdeada55d3d3382db240bed5ae832dc755f1dc"))
        (2275000, uint256("0x64d0f0ff26f0e955b42bb325d506ded0cb5393c0dc882ec5103c5114e3ed99a4"))
        (2300000, uint256("0xc73bdc59ae0ab897c668dea0550f467e4521caebb4abed00e2ddda66e07da236"))
        (2325000, uint256("0x4031fe8d9ab0fbca4f47d867af73e9f2be8779c6cc736a1b5fe272c0ff0eb241"))
        (2350000, uint256("0x03842281e12767766fd58d9e67067f1b7b933262f8b0afeb75054e7a381b9a7f"))
        (2375000, uint256("0xf1186ab15cab67a5029a57d474589df26b325313470aa616df98019db82a13c7"))
        (2400000, uint256("0x1377d727071219b9d07c164d4d47f315e00ec2c57682fc68c73268b11d6daf5b"))
        (2425000, uint256("0x98470293e1d73de3927a9f29e7f194a7e3cff410557714a0aa97f53b348fce09"))
        (2450000, uint256("0xa849b75bd144affcce4e7fb8001f74babb025fb32f487598f908b6e101974352"))
        (2475000, uint256("0xa001fe43bfcd1d1f25843969830edf8f80f84d68510248138fcc8b63143dd245"))
        (2500000, uint256("0x53d621ec70ae61a97a9ede5f25d24d40f52cfda4a2c514617ed0e5dd651c473e"))
        (2525000, uint256("0x3cbe6e4321c8e96def16cf867cda88adf01972a04b167f24ed26b4fe5bce8c7e"))
        (2550000, uint256("0xb96ec329c86b806e084304aa069f20ebffb816d5ecd7baf124ba1bc18919726a"))
        (2575000, uint256("0xcd6136d5189d69f4140568b3ecaac6108bd2aa5e28337865757ac29fef0c7457"))
        (2600000, uint256("0xb2c140d171353fbcfc82b80f6c1ee041e7137e6d06c76310336d61a710de1c03"))
        (2625000, uint256("0x0a911ca4f3704ce10d3306bb539587dbcc0adb2068a76c123e26144014b6aee2"))
        (2650000, uint256("0x5b87ec66086de7697c8144fb080075ecb042d1d455245536d7b084c44612c99a"))
        (2675000, uint256("0x79b7c06249f4562bd7e689bfbd726ffa3889054847f7c10295090e2429c2ac43"))
        (2700000, uint256("0xfb4e23eec825068ffd67af9ccadba135a7e9024d2fb88f2c62fd7071a5efcbbe"))
        (2725000, uint256("0xbee9640a0bdf5c55784440615a346af34a2cd3133d0755f26c533d4d4f4f9e39"))
        (2750000, uint256("0xf2d602108810c7b246c8bae49e11d70dc790c24f1b720aa4822bbf3a57e559e0"))
        (2775000, uint256("0x26d37f76c94f287ed5eefd13b05685b30cc4800439d8968bcdb13a391b9097a9"))
        (2800000, uint256("0x5eba97d9c892702de3a0258bb605eb1fee8346c4ae9c440afff7a0848bb76552"))
        (2825000, uint256("0x663b0b088f2620cd4d3b005ce48543da937c7f7bfe3f4972f0fd53b60ac81192"))
        (2850000, uint256("0xb703e60bfe8b4f1d1e9047fa012960ff11152b8c6e113101e952a185fec2a149"))
        (2875000, uint256("0xdaf9a60cf6429c4bece6fb9e6bd1548d5b496d471d505718285f53ff6a5fb376"))
        (2900000, uint256("0x1299c85a110652c78300315ad8d23fad7ee775452877e4ffd6789275e1845e53"))
        (2925000, uint256("0x17a7668edda0673c80810fe95db16a82162a60202e4610f69126b7fc7729d2e2"))
        (2950000, uint256("0x92f3fb5da5e5c2676cfeaa7f20c643843d314c68d2da888a06b6f203da69a4c2"))
        (2975000, uint256("0xcc455331ab5772ef0b8440474089002766af822b7f5a350750a62285e170e31d"))
        (3000000, uint256("0xecba35382007f16c83dba59e4dfe96881514e95157c4cb814e73213bc3cd2f09"))
        (3025000, uint256("0x52efdc6a2e95a7ae1df19f38066964dfd1edf2a1444dd4bc97860d01d87f2a6f"))
        (3050000, uint256("0x253bcac38c70deb6e5585d2f79b412b9928f1788222df7695d36113e51f1f36c"))
        (3075000, uint256("0xea3ed233aadda148df69f288a2a3bd42c00099342dd073273fbfbdae0d767d1f"))
        (3100000, uint256("0xa291699136be8acf6750d39682ddd8b09cfa6b72c38e68722475e549f224586c"))
        (3125000, uint256("0x21957b93d412e745b927f9d32b56e1349791d050f9c9c126c6209b099137c198"))
        (3150000, uint256("0xfbe484e8e13c9b495832a11604c0b7bd0d6ef5ae494ca74a456c14ebe105d693"))
        (3175000, uint256("0x1c5c31f153d501914e65741350605c9069eebf2ada2f3cb7e7e24ef2a5d9d6b8"))
        (3200000, uint256("0x5ce93625712e08fd831dc1e4d5db931edb9f2b864965647daab2f209c32cf456"))
        (3225000, uint256("0xaded7ec0bfd8e998b601bb9212a1760703daaa06826374c3a58571f65387f7b0"))
        (3250000, uint256("0xf4997e391f8d4a6566eae9c9d2c09ebcc8016ecb7470ce80eefdeef851a3b76a"))
        (3275000, uint256("0x2ae79adc3e48630c3ff38a576926174acf1696a7800448533e04d23c69ff9a62"))
        (3300000, uint256("0x8d4036ea2db964c3b9a79f65b63e1fd3e5e90da7fb8d409191e1761ace0cd82f"))
        (3325000, uint256("0xa722940d474e69dfb6e0b52d389431764a51a255fad436ab0b561650e654a6be"))
        (3350000, uint256("0x1fff80026b27d439d5b32cc7be3de49bbd817881e2bca6a17a5d27e297e0eb5e"))
        (3375000, uint256("0x82328e6b5a208c2a55e2602707c788b14597a78297aba8a26243d400fa2a4383"))
        (3400000, uint256("0x7350ca112c626fdec4a94d65a2c688261928687760117b53e5f7b593ec4febe9"))
        (3425000, uint256("0x441624bc41ac35982d6114ea384008b49aa1538ab9c62277cc23f768dc7859cc"))
        (3450000, uint256("0x58414881bfd813def83c2ebaba27ff5ef122a52e5f164b44128d70fe1b79dbf3"))
        (3475000, uint256("0xe475abf0ab6e6c4700d606185cf83b990b905aab23d795733152c8eef7f0a685"))
        (3500000, uint256("0x69f1db56bf0abf6f6ea8fbe1888840013a2bfa2ede0b55186bdf73ce8f686196"))
        (3525000, uint256("0xc70b070a31e03a75ed78757035f33cd9e00899aaebe8ff71d77333a244f5577a"))
        (3550000, uint256("0x08ebfd0e61536e3bc32ac9462b8f426193eccce1001674c674f263a10be0f58a"))
        (3575000, uint256("0xc26f675e6d65e5c48c3f3a3a3b55a138d20957f9b67dd1af7d82328ac87169c1"))
        (3600000, uint256("0xd326f60b4d0f08e373386c7bdb8c1e7936bba8e861c48bc57255bef9193e9172"))
        (3625000, uint256("0x01cf305905cde4f67501457b75c8e2fa43347b2dde8de0d80e3a2e2603b95865"))
        (3650000, uint256("0x3e52621ed18f822689092d61a8d0bf5a2d31b695ce0250f77b3b798f51108a03"))
        (3675000, uint256("0x6c0fa81f6001f559788b12a8e3e9324c27c17c16dcc9853418e9a09a6a31c039"))
        (3700000, uint256("0xd5e2f85395153fb0ff46c846b4a3e829b3613fce8d6262161bb4aa6526585162"))
        (3725000, uint256("0x78f6dd6227abb773c1efa3d91a0ff2e22b5f683d46a170c2748f70d2169fd9b5"))
        (3750000, uint256("0xc711113c02c92e1fc04af2dd3305971d2108fb8f6598a3ebfc506a7a1add9545"))
        (3775000, uint256("0xe1c710fe1a1ddddf1244375c7dd1ee2d7000ad63db7f87532bf1e18d69e91bfe"))
        (3800000, uint256("0x5b07cf0049b860d2119e0ed109f5aaaa794675edec50ab92ace1753526974c9c"))
        (3825000, uint256("0x04df338814793c4408ee770eef3a7a91b55594a47a306addee9417e1b653dba7"))
        (3850000, uint256("0xfbea160c8c0fb39e79026a09199bd017c155272b7ee5bfa3a28b0c368f1ff603"))
        (3875000, uint256("0xa566d34b3b757e09ea1d458a5748c23db1b5d9738874f696b6bd471741e5909a"))
        (3900000, uint256("0xfcc924390217b1fa0471fc58276b059cb777bb9f46467ae32bf3cbac2712897a"))
        (3925000, uint256("0xb4fa892c3d61e5197b2d3c1e9890b144e9779f86e18d04e5d88a979470d519d9"))
        (3950000, uint256("0x1e291e4477250fabeb8117f81f7e24bca0d0c0dfe2bdc68c3fdfa6f325d4e032"))
        (3975000, uint256("0xe0cbae4d30ef2aee8a7751e5f13b09dd670888c4b159867b10e578e6431ecd12"))
        (4000000, uint256("0xea6db9b18a31574dddc3ab3d7d10c3d1695daf342dd2558daed24df5c762d661"))
        (4025000, uint256("0x8d844660d8f0cf4e35a926231b0899835507b3d3d5944d7662509a6276b1bc9f"))
        (4050000, uint256("0x3b17617f243e36763a6bc54ab7c1aafc802b7a4c861554043441acefc66d74f9"))
        (4075000, uint256("0xb855bf4360617f5b662beb2932cd8fbf7a94f219ec6a29d9357736e48fd38b18"))
        (4100000, uint256("0xd1da6e3cff631db51d0f70e238956b2fd57e978009a45e02bb632f60b6a2b8d7"))
        (4125000, uint256("0x4df6df63a5468e78788192088a834406f020f1a00078017264566090ada87683"))
        (4150000, uint256("0xb6e460ce49d2f00771b9a578ed535a9bf1e918170e04657cb5b7260e3c0b6dc8"))
        (4175000, uint256("0x97469c52577c205810681d26bb1e99fa9e56b767461c6b2289a8945e653c3928"))
        (4200000, uint256("0xa2968f241aba6cb1ae9a8fe1467f1aab27673eb4e4f3d3da390929f7aca30f6c"))
        (4225000, uint256("0x806c9f5e91fcdaec9f596ea41e5d23217639a95985b04669d1637d7b9c275cb9"))
        (4250000, uint256("0x9341002b49f50bc6042923616c727cd3caa49998d3d872c825c6faced3669ac7"))
        (4275000, uint256("0x14b9e76f7ce0fa74f771ed3f8c3a000b42212a5d199642768667ed35a10a3b7c"))
        (4300000, uint256("0x3a21fca4f5e830ae843383de98d0f32329a6edcf9079889bc97c96e5c8b549ca"))
        (4325000, uint256("0x02d3a04e8ee6fc32e36b741e2c2e993a69523ca1899542b1e3c248a0e69c0ac8"))
        (4350000, uint256("0x39b53289e7661f85c85497badbacb08465391fd2ebd003ee4c4d54733e5dd558"))
        (4375000, uint256("0x8b920af94839862153b96d4e286659bdb2889ef09db3d702dc2f3f528117b56e"))
        (4400000, uint256("0xc2b6d727a64fe5e17c31f47c09adc1790a1f7132ef2304342693ff8fccf45f3a"))
        (4425000, uint256("0x41fc8477df2d13abb70a871c9ba5ebe7c529d648a8b720569b228f8a09e3439c"))
        ;

    // TestNet has no checkpoints
    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        ( 0, uint256("0xcc0e6b7fc1e4fa77e38306fd870bd0813178041cf9c1cafb6146c7d7fb8de492"))
        ;

    bool CheckHardened(int nHeight, const uint256& hash)
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    int GetTotalBlocksEstimate()
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

    // ppcoin: synchronized checkpoint (centrally broadcasted)
    uint256 hashSyncCheckpoint = 0;
    uint256 hashPendingCheckpoint = 0;
    CSyncCheckpoint checkpointMessage;
    CSyncCheckpoint checkpointMessagePending;
    uint256 hashInvalidCheckpoint = 0;
    CCriticalSection cs_hashSyncCheckpoint;

    // ppcoin: get last synchronized checkpoint
    CBlockIndex* GetLastSyncCheckpoint()
    {
        LOCK(cs_hashSyncCheckpoint);
        if (!mapBlockIndex.count(hashSyncCheckpoint))
            error("GetSyncCheckpoint: block index missing for current sync-checkpoint %s", hashSyncCheckpoint.ToString().c_str());
        else
            return mapBlockIndex[hashSyncCheckpoint];
        return NULL;
    }

    // ppcoin: only descendant of current sync-checkpoint is allowed
    bool ValidateSyncCheckpoint(uint256 hashCheckpoint)
    {
        if (!mapBlockIndex.count(hashSyncCheckpoint))
            return error("ValidateSyncCheckpoint: block index missing for current sync-checkpoint %s", hashSyncCheckpoint.ToString().c_str());
        if (!mapBlockIndex.count(hashCheckpoint))
            return error("ValidateSyncCheckpoint: block index missing for received sync-checkpoint %s", hashCheckpoint.ToString().c_str());

        CBlockIndex* pindexSyncCheckpoint = mapBlockIndex[hashSyncCheckpoint];
        CBlockIndex* pindexCheckpointRecv = mapBlockIndex[hashCheckpoint];

        if (pindexCheckpointRecv->nHeight <= pindexSyncCheckpoint->nHeight)
        {
            // Received an older checkpoint, trace back from current checkpoint
            // to the same height of the received checkpoint to verify
            // that current checkpoint should be a descendant block
            CBlockIndex* pindex = pindexSyncCheckpoint;
            while (pindex->nHeight > pindexCheckpointRecv->nHeight)
                if (!(pindex = pindex->pprev))
                    return error("ValidateSyncCheckpoint: pprev null - block index structure failure");
            if (pindex->GetBlockHash() != hashCheckpoint)
            {
                hashInvalidCheckpoint = hashCheckpoint;
                return error("ValidateSyncCheckpoint: new sync-checkpoint %s is conflicting with current sync-checkpoint %s", hashCheckpoint.ToString().c_str(), hashSyncCheckpoint.ToString().c_str());
            }
            return false; // ignore older checkpoint
        }

        // Received checkpoint should be a descendant block of the current
        // checkpoint. Trace back to the same height of current checkpoint
        // to verify.
        CBlockIndex* pindex = pindexCheckpointRecv;
        while (pindex->nHeight > pindexSyncCheckpoint->nHeight)
            if (!(pindex = pindex->pprev))
                return error("ValidateSyncCheckpoint: pprev2 null - block index structure failure");
        if (pindex->GetBlockHash() != hashSyncCheckpoint)
        {
            hashInvalidCheckpoint = hashCheckpoint;
            return error("ValidateSyncCheckpoint: new sync-checkpoint %s is not a descendant of current sync-checkpoint %s", hashCheckpoint.ToString().c_str(), hashSyncCheckpoint.ToString().c_str());
        }
        return true;
    }

    bool WriteSyncCheckpoint(const uint256& hashCheckpoint)
    {
        CTxDB txdb;
        txdb.TxnBegin();
        if (!txdb.WriteSyncCheckpoint(hashCheckpoint))
        {
            txdb.TxnAbort();
            return error("WriteSyncCheckpoint(): failed to write to db sync checkpoint %s", hashCheckpoint.ToString().c_str());
        }
        if (!txdb.TxnCommit())
            return error("WriteSyncCheckpoint(): failed to commit to db sync checkpoint %s", hashCheckpoint.ToString().c_str());

        Checkpoints::hashSyncCheckpoint = hashCheckpoint;
        return true;
    }

    bool AcceptPendingSyncCheckpoint()
    {
        LOCK(cs_hashSyncCheckpoint);
        if (hashPendingCheckpoint != 0 && mapBlockIndex.count(hashPendingCheckpoint))
        {
            if (!ValidateSyncCheckpoint(hashPendingCheckpoint))
            {
                hashPendingCheckpoint = 0;
                checkpointMessagePending.SetNull();
                return false;
            }

            CTxDB txdb;
            CBlockIndex* pindexCheckpoint = mapBlockIndex[hashPendingCheckpoint];
            if (!pindexCheckpoint->IsInMainChain())
            {
                CBlock block;
                if (!block.ReadFromDisk(pindexCheckpoint))
                    return error("AcceptPendingSyncCheckpoint: ReadFromDisk failed for sync checkpoint %s", hashPendingCheckpoint.ToString().c_str());
                if (!block.SetBestChain(txdb, pindexCheckpoint))
                {
                    hashInvalidCheckpoint = hashPendingCheckpoint;
                    return error("AcceptPendingSyncCheckpoint: SetBestChain failed for sync checkpoint %s", hashPendingCheckpoint.ToString().c_str());
                }
            }

            if (!WriteSyncCheckpoint(hashPendingCheckpoint))
                return error("AcceptPendingSyncCheckpoint(): failed to write sync checkpoint %s", hashPendingCheckpoint.ToString().c_str());
            hashPendingCheckpoint = 0;
            checkpointMessage = checkpointMessagePending;
            checkpointMessagePending.SetNull();
            printf("AcceptPendingSyncCheckpoint : sync-checkpoint at %s\n", hashSyncCheckpoint.ToString().c_str());
            // relay the checkpoint
            if (!checkpointMessage.IsNull())
            {
                BOOST_FOREACH(CNode* pnode, vNodes)
                    checkpointMessage.RelayTo(pnode);
            }
            return true;
        }
        return false;
    }

    // Automatically select a suitable sync-checkpoint
    uint256 AutoSelectSyncCheckpoint()
    {
        const CBlockIndex *pindex = pindexBest;
        // Search backward for a block within max span and maturity window
        while (pindex->pprev && (pindex->GetBlockTime() + nCheckpointSpan * nTargetSpacing > pindexBest->GetBlockTime() || pindex->nHeight + nCheckpointSpan > pindexBest->nHeight))
            pindex = pindex->pprev;
        return pindex->GetBlockHash();
    }

    // Check against synchronized checkpoint
    bool CheckSync(const uint256& hashBlock, const CBlockIndex* pindexPrev)
    {
        if (TestNet()) return true; // Testnet has no checkpoints
        int nHeight = pindexPrev->nHeight + 1;

        LOCK(cs_hashSyncCheckpoint);
        // sync-checkpoint should always be accepted block
        assert(mapBlockIndex.count(hashSyncCheckpoint));
        const CBlockIndex* pindexSync = mapBlockIndex[hashSyncCheckpoint];

        if (nHeight > pindexSync->nHeight)
        {
            // Fast path: when both the sync-checkpoint and the new block's parent
            // are on our main chain, the parent descends from the checkpoint (the
            // main chain is a single line). Without this, every new block walked
            // back to the checkpoint one block at a time; with the checkpoint at
            // genesis that was millions of steps (about 0.25 s) per block.
            if (pindexSync->IsInMainChain() && pindexPrev->IsInMainChain())
                return true;

            // trace back to same height as sync-checkpoint
            const CBlockIndex* pindex = pindexPrev;
            while (pindex->nHeight > pindexSync->nHeight)
                if (!(pindex = pindex->pprev))
                    return error("CheckSync: pprev null - block index structure failure");
            if (pindex->nHeight < pindexSync->nHeight || pindex->GetBlockHash() != hashSyncCheckpoint)
                return false; // only descendant of sync-checkpoint can pass check
        }
        if (nHeight == pindexSync->nHeight && hashBlock != hashSyncCheckpoint)
            return false; // same height with sync-checkpoint
        if (nHeight < pindexSync->nHeight && !mapBlockIndex.count(hashBlock))
            return false; // lower height than sync-checkpoint
        return true;
    }

    bool WantedByPendingSyncCheckpoint(uint256 hashBlock)
    {
        LOCK(cs_hashSyncCheckpoint);
        if (hashPendingCheckpoint == 0)
            return false;
        if (hashBlock == hashPendingCheckpoint)
            return true;
        if (mapOrphanBlocks.count(hashPendingCheckpoint)
            && hashBlock == WantedByOrphan(mapOrphanBlocks[hashPendingCheckpoint]))
            return true;
        return false;
    }

    // ppcoin: reset synchronized checkpoint to last hardened checkpoint
    bool ResetSyncCheckpoint()
    {
        LOCK(cs_hashSyncCheckpoint);
        const uint256& hash = mapCheckpoints.rbegin()->second;
        if (mapBlockIndex.count(hash) && !mapBlockIndex[hash]->IsInMainChain())
        {
            // checkpoint block accepted but not yet in main chain
            printf("ResetSyncCheckpoint: SetBestChain to hardened checkpoint %s\n", hash.ToString().c_str());
            CTxDB txdb;
            CBlock block;
            if (!block.ReadFromDisk(mapBlockIndex[hash]))
                return error("ResetSyncCheckpoint: ReadFromDisk failed for hardened checkpoint %s", hash.ToString().c_str());
            if (!block.SetBestChain(txdb, mapBlockIndex[hash]))
            {
                return error("ResetSyncCheckpoint: SetBestChain failed for hardened checkpoint %s", hash.ToString().c_str());
            }
        }
        else if(!mapBlockIndex.count(hash))
        {
            // checkpoint block not yet accepted
            hashPendingCheckpoint = hash;
            checkpointMessagePending.SetNull();
            printf("ResetSyncCheckpoint: pending for sync-checkpoint %s\n", hashPendingCheckpoint.ToString().c_str());
        }

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, mapCheckpoints)
        {
            const uint256& hash = i.second;
            if (mapBlockIndex.count(hash) && mapBlockIndex[hash]->IsInMainChain())
            {
                if (!WriteSyncCheckpoint(hash))
                    return error("ResetSyncCheckpoint: failed to write sync checkpoint %s", hash.ToString().c_str());
                printf("ResetSyncCheckpoint: sync-checkpoint reset to %s\n", hashSyncCheckpoint.ToString().c_str());
                return true;
            }
        }

        return false;
    }

    void AskForPendingSyncCheckpoint(CNode* pfrom)
    {
        LOCK(cs_hashSyncCheckpoint);
        if (pfrom && hashPendingCheckpoint != 0 && (!mapBlockIndex.count(hashPendingCheckpoint)) && (!mapOrphanBlocks.count(hashPendingCheckpoint)))
            pfrom->AskFor(CInv(MSG_BLOCK, hashPendingCheckpoint));
    }

    bool SetCheckpointPrivKey(std::string strPrivKey)
    {
        // Test signing a sync-checkpoint with genesis block
        CSyncCheckpoint checkpoint;
        // checkpoint.hashCheckpoint = (!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet);
        checkpoint.hashCheckpoint = Params().HashGenesisBlock();
        CDataStream sMsg(SER_NETWORK, PROTOCOL_VERSION);
        sMsg << (CUnsignedSyncCheckpoint)checkpoint;
        checkpoint.vchMsg = std::vector<unsigned char>(sMsg.begin(), sMsg.end());

        std::vector<unsigned char> vchPrivKey = ParseHex(strPrivKey);
        CKey key;
        key.SetPrivKey(CPrivKey(vchPrivKey.begin(), vchPrivKey.end()), false); // if key is not correct openssl may crash
        if (!key.Sign(Hash(checkpoint.vchMsg.begin(), checkpoint.vchMsg.end()), checkpoint.vchSig))
            return false;

        // Test signing successful, proceed
        CSyncCheckpoint::strMasterPrivKey = strPrivKey;
        return true;
    }

    bool SendSyncCheckpoint(uint256 hashCheckpoint)
    {
        CSyncCheckpoint checkpoint;
        checkpoint.hashCheckpoint = hashCheckpoint;
        CDataStream sMsg(SER_NETWORK, PROTOCOL_VERSION);
        sMsg << (CUnsignedSyncCheckpoint)checkpoint;
        checkpoint.vchMsg = std::vector<unsigned char>(sMsg.begin(), sMsg.end());

        if (CSyncCheckpoint::strMasterPrivKey.empty())
            return error("SendSyncCheckpoint: Checkpoint master key unavailable.");
        std::vector<unsigned char> vchPrivKey = ParseHex(CSyncCheckpoint::strMasterPrivKey);
        CKey key;
        key.SetPrivKey(CPrivKey(vchPrivKey.begin(), vchPrivKey.end()), false); // if key is not correct openssl may crash
        if (!key.Sign(Hash(checkpoint.vchMsg.begin(), checkpoint.vchMsg.end()), checkpoint.vchSig))
            return error("SendSyncCheckpoint: Unable to sign checkpoint, check private key?");

        if(!checkpoint.ProcessSyncCheckpoint(NULL))
        {
            printf("WARNING: SendSyncCheckpoint: Failed to process checkpoint.\n");
            return false;
        }

        // Relay checkpoint
        {
            LOCK(cs_vNodes);
            BOOST_FOREACH(CNode* pnode, vNodes)
                checkpoint.RelayTo(pnode);
        }
        return true;
    }

    // Is the sync-checkpoint outside maturity window?
    bool IsMatureSyncCheckpoint()
    {
        LOCK(cs_hashSyncCheckpoint);
        // sync-checkpoint should always be accepted block
        assert(mapBlockIndex.count(hashSyncCheckpoint));
        const CBlockIndex* pindexSync = mapBlockIndex[hashSyncCheckpoint];
        return (nBestHeight >= pindexSync->nHeight + nCoinbaseMaturity ||
                pindexSync->GetBlockTime() + nStakeMinAge < GetAdjustedTime());
    }
}

// flapx: sync-checkpoint master key
const std::string CSyncCheckpoint::strMasterPubKey = "046b2e58c8c0502bea592b7ec55f5acb8781a2764520f346642a5c2a6e8ef8b00704752e825d9e739daac343fae5027945e018a15e456e98ef2d0b507fbd9a6d13";

std::string CSyncCheckpoint::strMasterPrivKey = "";

bool CSyncCheckpoint::RelayTo(CNode* pnode) const
{
    // don't relay to nodes which haven't sent their version message
    if (pnode->nVersion == 0)
        return false;
    // returns true if wasn't already sent
    if (pnode->hashCheckpointKnown != hashCheckpoint)
    {
        pnode->hashCheckpointKnown = hashCheckpoint;
        pnode->PushMessage("checkpoint", *this);
        return true;
    }
    return false;
}

// ppcoin: verify signature of sync-checkpoint message
bool CSyncCheckpoint::CheckSignature()
{
    // CKey key;
    // if (!key.SetPubKey(ParseHex(CSyncCheckpoint::strMasterPubKey)))
    //     return error("CSyncCheckpoint::CheckSignature() : SetPubKey failed");
    CPubKey key(ParseHex(CSyncCheckpoint::strMasterPubKey));
    if (!key.Verify(Hash(vchMsg.begin(), vchMsg.end()), vchSig))
        return error("CSyncCheckpoint::CheckSignature() : verify signature failed");

    // Now unserialize the data
    CDataStream sMsg(vchMsg, SER_NETWORK, PROTOCOL_VERSION);
    sMsg >> *(CUnsignedSyncCheckpoint*)this;
    return true;
}

// ppcoin: process synchronized checkpoint
bool CSyncCheckpoint::ProcessSyncCheckpoint(CNode* pfrom)
{
    if (!CheckSignature())
        return false;

    LOCK(Checkpoints::cs_hashSyncCheckpoint);
    if (!mapBlockIndex.count(hashCheckpoint))
    {
        // We haven't received the checkpoint chain, keep the checkpoint as pending
        Checkpoints::hashPendingCheckpoint = hashCheckpoint;
        Checkpoints::checkpointMessagePending = *this;
        printf("ProcessSyncCheckpoint: pending for sync-checkpoint %s\n", hashCheckpoint.ToString().c_str());
        // Ask this guy to fill in what we're missing
        if (pfrom)
        {
            // pfrom->PushGetBlocks(pindexBest, hashCheckpoint);
            PushGetBlocks(pfrom, pindexBest, hashCheckpoint);
            // ask directly as well in case rejected earlier by duplicate
            // proof-of-stake because getblocks may not get it this time
            pfrom->AskFor(CInv(MSG_BLOCK, mapOrphanBlocks.count(hashCheckpoint)? WantedByOrphan(mapOrphanBlocks[hashCheckpoint]) : hashCheckpoint));
        }
        return false;
    }

    if (!Checkpoints::ValidateSyncCheckpoint(hashCheckpoint))
        return false;

    CTxDB txdb;
    CBlockIndex* pindexCheckpoint = mapBlockIndex[hashCheckpoint];
    if (!pindexCheckpoint->IsInMainChain())
    {
        // checkpoint chain received but not yet main chain
        CBlock block;
        if (!block.ReadFromDisk(pindexCheckpoint))
            return error("ProcessSyncCheckpoint: ReadFromDisk failed for sync checkpoint %s", hashCheckpoint.ToString().c_str());
        if (!block.SetBestChain(txdb, pindexCheckpoint))
        {
            Checkpoints::hashInvalidCheckpoint = hashCheckpoint;
            return error("ProcessSyncCheckpoint: SetBestChain failed for sync checkpoint %s", hashCheckpoint.ToString().c_str());
        }
    }

    if (!Checkpoints::WriteSyncCheckpoint(hashCheckpoint))
        return error("ProcessSyncCheckpoint(): failed to write sync checkpoint %s", hashCheckpoint.ToString().c_str());
    Checkpoints::checkpointMessage = *this;
    Checkpoints::hashPendingCheckpoint = 0;
    Checkpoints::checkpointMessagePending.SetNull();
    printf("ProcessSyncCheckpoint: sync-checkpoint at %s\n", hashCheckpoint.ToString().c_str());
    return true;
}
