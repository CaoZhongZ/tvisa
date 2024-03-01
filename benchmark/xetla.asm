//.kernel _ZTSZZ16basic_brgemm_runjENKUlRN4sycl3_V17handlerEE_clES2_EUlNS0_7nd_itemILi3EEEE_
//.platform PVCXT
//.thread_config numGRF=256, numAcc=8, numSWSB=32
//.options_string "-enableHalfLSC -printregusage -DPASTokenReduction -enableBCR -dumpcommonisa -output -binary -TotalGRFNum 256 -abiver 2 "
//.full_options "-abiver 2 -printregusage -TotalGRFNum 256 -enableBCR -DPASTokenReduction -output -binary -dumpcommonisa -enableHalfLSC "
//.instCount 355
//.RA type	TRIVIAL_BC_RA
//.git-hash 653e58190ca4b073722bb355669a540de79bdf5d
//.GRF count 240

//.declare BuiltInR0 (0)  rf=r size=64 type=ud align=32 words (r0.0) IsBuiltin
//.declare  (1)  rf=r size=64 type=ud alias=BuiltInR0+0 align=32 words (r0.0) IsBuiltin
//.declare BuiltinA0 (2)  rf=a size=4 type=ud align=1 words (a0.0) IsBuiltin
//.declare BuiltinA0Dot2 (3)  rf=a size=4 type=ud align=1 words (a0.2) IsBuiltin
//.declare %null (9)  rf=r size=4 type=ud align=2 words
//.declare %local_id_x (12)  rf=r size=4 type=ud align=2 words (r3.7)
//.declare %local_id_y (13)  rf=r size=4 type=ud align=2 words (r3.8)
//.declare %local_size_x (14)  rf=r size=4 type=ud align=2 words (r3.3)
//.declare %local_size_y (15)  rf=r size=4 type=ud align=2 words (r3.4)
//.declare %group_id_x (16)  rf=r size=4 type=ud align=2 words (r0.1)
//.declare %group_id_y (17)  rf=r size=4 type=ud align=2 words (r0.6)
//.declare %group_id_z (18)  rf=r size=4 type=ud align=2 words (r0.7)
//.declare %group_count_x (19)  rf=r size=4 type=ud align=2 words (r3.5)
//.declare %group_count_y (20)  rf=r size=4 type=ud align=2 words (r3.6)
//.declare %tsc (21)  rf=r size=20 type=ud align=2 words
//.declare %arg (22)  rf=r size=0 type=ud align=32 words (r26.0)
//.declare %retval (23)  rf=r size=0 type=ud align=32 words (r26.0) Output
//.declare %sp (24)  rf=r size=8 type=uq align=4 words (r255.3)
//.declare %fp (25)  rf=r size=8 type=uq align=4 words (r255.2)
//.declare %sr0 (26)  rf=r size=16 type=ud align=2 words
//.declare %cr0 (27)  rf=r size=12 type=ud align=2 words
//.declare %ce0 (28)  rf=r size=4 type=ud align=2 words
//.declare %dbg0 (29)  rf=r size=8 type=ud align=2 words
//.declare implBufPtr (31)  rf=r size=8 type=uq align=4 words (r254.0)
//.declare localIdBufPtr (32)  rf=r size=8 type=uq align=4 words (r254.3)
//.declare %msg0 (33)  rf=r size=12 type=ud align=2 words
//.declare V32 (38)  rf=r size=4 type=d align=2 words (r2.6)
//.declare V33 (39)  rf=r size=4 type=d align=2 words (r2.10)
//.declare V34 (40)  rf=r size=4 type=d align=2 words (r2.11)
//.declare V35 (41)  rf=r size=4 type=d align=2 words (r2.14)
//.declare V36 (42)  rf=r size=4 type=d align=2 words (r2.15)
//.declare V37 (43)  rf=r size=4 type=d align=2 words (r3.2)
//.declare V38 (44)  rf=r size=6 type=w align=1 words (r1.0)
//.declare V39 (45)  rf=r size=12 type=d align=2 words (r2.0)
//.declare V40 (46)  rf=r size=8 type=q align=4 words (r2.2)
//.declare V42 (48)  rf=r size=4 type=d align=2 words (r1.2)
//.declare V43 (49)  rf=r size=8 type=d align=2 words (r1.3)
//.declare V44 (50)  rf=r size=12 type=d align=2 words (r1.5)
//.declare V45 (51)  rf=r size=8 type=d align=32 words (r4.0)
//.declare V46 (52)  rf=r size=12 type=d align=2 words (r1.8)
//.declare V47 (53)  rf=r size=8 type=uq align=4 words (r2.4)
//.declare V48 (54)  rf=r size=4 type=d align=2 words (r1.11)
//.declare V49 (55)  rf=r size=4 type=d align=2 words (r1.12)
//.declare V50 (56)  rf=r size=4 type=d align=2 words (r1.13)
//.declare V51 (57)  rf=r size=4 type=d align=2 words (r1.14)
//.declare V52 (58)  rf=r size=4 type=d align=2 words (r1.15)
//.declare V53 (59)  rf=r size=8 type=uq align=4 words (r2.6)
//.declare V54 (60)  rf=r size=4 type=d align=2 words (r2.3)
//.declare V55 (61)  rf=r size=4 type=d align=2 words (r2.4)
//.declare V56 (62)  rf=r size=4 type=d align=2 words (r2.5)
//.declare V57 (63)  rf=r size=4 type=d align=2 words (r2.7)
//.declare V58 (64)  rf=r size=4 type=d align=2 words (r3.3)
//.declare P1 (65)  rf=f1  size=2 type=uw align=1 words (f0.1)
//.declare V59 (66)  rf=r size=4 type=d align=2 words (r3.4)
//.declare V60 (67)  rf=r size=64 type=d align=32 words (r5.0)
//.declare V61 (68)  rf=r size=64 type=d align=32 words (r6.0)
//.declare V62 (69)  rf=r size=64 type=d align=32 words (r7.0)
//.declare V63 (70)  rf=r size=64 type=d align=32 words (r8.0)
//.declare V64 (71)  rf=r size=64 type=d align=32 words (r9.0)
//.declare V65 (72)  rf=r size=64 type=d align=32 words (r10.0)
//.declare V66 (73)  rf=r size=1024 type=w align=32 words (r11.0)
//.declare V67 (74)  rf=r size=1024 type=w align=32 words (r27.0)
//.declare V68 (75)  rf=r size=512 type=w align=32 words (r43.0)
//.declare V69 (76)  rf=r size=512 type=w align=32 words (r51.0)
//.declare V70 (77)  rf=r size=512 type=d align=32 words (r59.0)
//.declare V71 (78)  rf=r size=512 type=d align=32 words (r67.0)
//.declare V72 (79)  rf=r size=512 type=d align=32 words (r75.0)
//.declare V73 (80)  rf=r size=512 type=d align=32 words (r83.0)
//.declare V74 (81)  rf=r size=256 type=d align=32 words (r95.0)
//.declare V75 (82)  rf=r size=256 type=d align=32 words (r103.0)
//.declare V76 (83)  rf=r size=256 type=d align=32 words (r111.0)
//.declare V77 (84)  rf=r size=256 type=d align=32 words (r119.0)
//.declare P2 (85)  rf=f1  size=2 type=uw align=1 words (f0.0)
//.declare V78 (86)  rf=r size=256 type=hf align=32 words (r126.0)
//.declare V79 (87)  rf=r size=256 type=hf align=32 words (r130.0)
//.declare V80 (88)  rf=r size=256 type=hf align=32 words (r134.0)
//.declare V81 (89)  rf=r size=256 type=hf align=32 words (r138.0)
//.declare V82 (90)  rf=r size=256 type=hf align=32 words (r142.0)
//.declare V83 (91)  rf=r size=256 type=hf align=32 words (r146.0)
//.declare V84 (92)  rf=r size=256 type=hf align=32 words (r150.0)
//.declare V85 (93)  rf=r size=256 type=hf align=32 words (r154.0)
//.declare V86 (94)  rf=r size=256 type=hf align=32 words (r158.0)
//.declare V87 (95)  rf=r size=256 type=hf align=32 words (r162.0)
//.declare V88 (96)  rf=r size=256 type=hf align=32 words (r166.0)
//.declare V89 (97)  rf=r size=256 type=hf align=32 words (r170.0)
//.declare V90 (98)  rf=r size=256 type=hf align=32 words (r174.0)
//.declare V91 (99)  rf=r size=256 type=hf align=32 words (r178.0)
//.declare V92 (100)  rf=r size=256 type=hf align=32 words (r182.0)
//.declare V93 (101)  rf=r size=256 type=hf align=32 words (r186.0)
//.declare V94 (102)  rf=r size=4 type=d align=2 words (r3.5)
//.declare V95 (103)  rf=r size=4 type=d align=2 words (r3.6)
//.declare V96 (104)  rf=r size=8 type=uq align=4 words (r3.0)
//.declare V97 (105)  rf=r size=4 type=d align=2 words (r3.7)
//.declare V98 (106)  rf=r size=512 type=w align=32 words (r190.0)
//.declare V99 (107)  rf=r size=512 type=w align=32 words (r198.0)
//.declare V100 (108)  rf=r size=64 type=d align=32 words (r91.0)
//.declare V101 (109)  rf=r size=512 type=w align=32 words (r206.0)
//.declare V102 (110)  rf=r size=512 type=w align=32 words (r214.0)
//.declare V103 (111)  rf=r size=64 type=d align=32 words (r92.0)
//.declare V104 (112)  rf=r size=512 type=w align=32 words (r222.0)
//.declare V105 (113)  rf=r size=512 type=w align=32 words (r230.0)
//.declare V106 (114)  rf=r size=64 type=d align=32 words (r93.0)
//.declare V107 (115)  rf=r size=512 type=w align=32 words (r238.0)
//.declare V108 (116)  rf=r size=512 type=w align=32 words (r246.0)
//.declare V109 (117)  rf=r size=64 type=d align=32 words (r94.0)
//.declare V110 (118)  rf=r size=6 type=uw alias=V38+0 align=1 words (r1.0)
//.declare V111 (119)  rf=r size=4 type=d alias=V42+0 align=2 words (r1.2)
//.declare V112 (120)  rf=r size=8 type=d alias=V43+0 align=2 words (r1.3)
//.declare V114 (122)  rf=r size=12 type=d alias=V39+0 align=2 words (r2.0)
//.declare V115 (123)  rf=r size=12 type=d alias=V44+0 align=2 words (r1.5)
//.declare V116 (124)  rf=r size=8 type=d alias=V45+0 align=2 words (r4.0)
//.declare V117 (125)  rf=r size=12 type=d alias=V46+0 align=2 words (r1.8)
//.declare V118 (126)  rf=r size=4 type=ud alias=V58+0 align=2 words (r3.3)
//.declare V119 (127)  rf=r size=12 type=ud alias=V46+0 align=2 words (r1.8)
//.declare V120 (128)  rf=r size=4 type=ud alias=V57+0 align=2 words (r2.7)
//.declare V121 (129)  rf=r size=8 type=q alias=V47+0 align=4 words (r2.4)
//.declare V122 (130)  rf=r size=64 type=q alias=V60+0 align=32 words (r5.0)
//.declare V123 (131)  rf=r size=4 type=d alias=V48+0 align=2 words (r1.11)
//.declare V124 (132)  rf=r size=4 type=d alias=V32+0 align=2 words (r2.6)
//.declare V125 (133)  rf=r size=64 type=d alias=V60+0 align=32 words (r5.0)
//.declare V126 (134)  rf=r size=4 type=d alias=V33+0 align=2 words (r2.10)
//.declare V127 (135)  rf=r size=4 type=d alias=V49+0 align=2 words (r1.12)
//.declare V128 (136)  rf=r size=4 type=d alias=V34+0 align=2 words (r2.11)
//.declare V129 (137)  rf=r size=4 type=d alias=V57+0 align=2 words (r2.7)
//.declare V130 (138)  rf=r size=4 type=d alias=V50+0 align=2 words (r1.13)
//.declare V131 (139)  rf=r size=4 type=ud alias=V51+0 align=2 words (r1.14)
//.declare V132 (140)  rf=r size=4 type=ud alias=V32+0 align=2 words (r2.6)
//.declare V133 (141)  rf=r size=4 type=d alias=V56+0 align=2 words (r2.5)
//.declare V134 (142)  rf=r size=4 type=d alias=V52+0 align=2 words (r1.15)
//.declare V135 (143)  rf=r size=8 type=q alias=V53+0 align=4 words (r2.6)
//.declare V136 (144)  rf=r size=64 type=q alias=V62+0 align=32 words (r7.0)
//.declare V137 (145)  rf=r size=4 type=d alias=V54+0 align=2 words (r2.3)
//.declare V138 (146)  rf=r size=4 type=d alias=V35+0 align=2 words (r2.14)
//.declare V139 (147)  rf=r size=64 type=d alias=V62+0 align=32 words (r7.0)
//.declare V140 (148)  rf=r size=64 type=ud alias=V62+0 align=32 words (r7.0)
//.declare V141 (149)  rf=r size=4 type=d alias=V55+0 align=2 words (r2.4)
//.declare V142 (150)  rf=r size=4 type=d alias=V36+0 align=2 words (r2.15)
//.declare V143 (151)  rf=r size=64 type=d alias=V64+0 align=32 words (r9.0)
//.declare V144 (152)  rf=r size=64 type=d alias=V65+0 align=32 words (r10.0)
//.declare V145 (153)  rf=r size=64 type=ud alias=V65+0 align=32 words (r10.0)
//.declare V146 (154)  rf=r size=256 type=d alias=V78+0 align=32 words (r126.0)
//.declare V147 (155)  rf=r size=256 type=d alias=V79+0 align=32 words (r130.0)
//.declare V148 (156)  rf=r size=256 type=d alias=V80+0 align=32 words (r134.0)
//.declare V149 (157)  rf=r size=256 type=d alias=V81+0 align=32 words (r138.0)
//.declare V150 (158)  rf=r size=256 type=d alias=V82+0 align=32 words (r142.0)
//.declare V151 (159)  rf=r size=256 type=d alias=V83+0 align=32 words (r146.0)
//.declare V152 (160)  rf=r size=256 type=d alias=V84+0 align=32 words (r150.0)
//.declare V153 (161)  rf=r size=256 type=d alias=V85+0 align=32 words (r154.0)
//.declare V154 (162)  rf=r size=256 type=d alias=V86+0 align=32 words (r158.0)
//.declare V155 (163)  rf=r size=256 type=d alias=V87+0 align=32 words (r162.0)
//.declare V156 (164)  rf=r size=256 type=d alias=V88+0 align=32 words (r166.0)
//.declare V157 (165)  rf=r size=256 type=d alias=V89+0 align=32 words (r170.0)
//.declare V158 (166)  rf=r size=256 type=d alias=V90+0 align=32 words (r174.0)
//.declare V159 (167)  rf=r size=256 type=d alias=V91+0 align=32 words (r178.0)
//.declare V160 (168)  rf=r size=256 type=d alias=V92+0 align=32 words (r182.0)
//.declare V161 (169)  rf=r size=256 type=d alias=V93+0 align=32 words (r186.0)
//.declare V162 (170)  rf=r size=64 type=d alias=V63+0 align=32 words (r8.0)
//.declare V163 (171)  rf=r size=64 type=d alias=V61+0 align=32 words (r6.0)
//.declare V164 (172)  rf=r size=512 type=w alias=V70+0 align=32 words (r59.0)
//.declare V165 (173)  rf=r size=512 type=w alias=V71+0 align=32 words (r67.0)
//.declare V166 (174)  rf=r size=256 type=w alias=V74+0 align=32 words (r95.0)
//.declare V167 (175)  rf=r size=256 type=w alias=V75+0 align=32 words (r103.0)
//.declare V168 (176)  rf=r size=256 type=w alias=V76+0 align=32 words (r111.0)
//.declare V169 (177)  rf=r size=256 type=w alias=V77+0 align=32 words (r119.0)
//.declare V170 (178)  rf=r size=512 type=w alias=V72+0 align=32 words (r75.0)
//.declare V171 (179)  rf=r size=512 type=w alias=V73+0 align=32 words (r83.0)
//.declare V172 (180)  rf=r size=256 type=hf alias=V78+0 align=32 words (r126.0)
//.declare V173 (181)  rf=r size=256 type=hf alias=V79+0 align=32 words (r130.0)
//.declare V174 (182)  rf=r size=256 type=hf alias=V86+0 align=32 words (r158.0)
//.declare V175 (183)  rf=r size=256 type=hf alias=V87+0 align=32 words (r162.0)
//.declare V176 (184)  rf=r size=256 type=hf alias=V80+0 align=32 words (r134.0)
//.declare V177 (185)  rf=r size=256 type=hf alias=V81+0 align=32 words (r138.0)
//.declare V178 (186)  rf=r size=256 type=hf alias=V88+0 align=32 words (r166.0)
//.declare V179 (187)  rf=r size=256 type=hf alias=V89+0 align=32 words (r170.0)
//.declare V180 (188)  rf=r size=256 type=hf alias=V82+0 align=32 words (r142.0)
//.declare V181 (189)  rf=r size=256 type=hf alias=V83+0 align=32 words (r146.0)
//.declare V182 (190)  rf=r size=256 type=hf alias=V90+0 align=32 words (r174.0)
//.declare V183 (191)  rf=r size=256 type=hf alias=V91+0 align=32 words (r178.0)
//.declare V184 (192)  rf=r size=256 type=hf alias=V84+0 align=32 words (r150.0)
//.declare V185 (193)  rf=r size=256 type=hf alias=V85+0 align=32 words (r154.0)
//.declare V186 (194)  rf=r size=256 type=hf alias=V92+0 align=32 words (r182.0)
//.declare V187 (195)  rf=r size=256 type=hf alias=V93+0 align=32 words (r186.0)
//.declare V188 (196)  rf=r size=4 type=d alias=V59+0 align=2 words (r3.4)
//.declare V189 (197)  rf=r size=4 type=d alias=V94+0 align=2 words (r3.5)
//.declare V190 (198)  rf=r size=4 type=d alias=V95+0 align=2 words (r3.6)
//.declare V191 (199)  rf=r size=8 type=q alias=V96+0 align=4 words (r3.0)
//.declare V192 (200)  rf=r size=64 type=q alias=V109+0 align=32 words (r94.0)
//.declare V193 (201)  rf=r size=64 type=d alias=V109+0 align=32 words (r94.0)
//.declare V194 (202)  rf=r size=4 type=d alias=V97+0 align=2 words (r3.7)
//.declare V195 (203)  rf=r size=4 type=d alias=V37+0 align=2 words (r3.2)
//.declare V196 (204)  rf=r size=64 type=ud alias=V109+0 align=32 words (r94.0)
//.declare V197 (205)  rf=r size=4 type=ud alias=V94+0 align=2 words (r3.5)
//.declare V198 (206)  rf=r size=4 type=ud alias=V50+0 align=2 words (r1.13)
//.declare V199 (207)  rf=r size=64 type=d alias=V106+0 align=32 words (r93.0)
//.declare V200 (208)  rf=r size=512 type=hf alias=V98+0 align=32 words (r190.0)
//.declare V201 (209)  rf=r size=512 type=hf alias=V99+0 align=32 words (r198.0)
//.declare V202 (210)  rf=r size=64 type=d alias=V100+0 align=32 words (r91.0)
//.declare V203 (211)  rf=r size=512 type=hf alias=V101+0 align=32 words (r206.0)
//.declare V204 (212)  rf=r size=512 type=hf alias=V102+0 align=32 words (r214.0)
//.declare V205 (213)  rf=r size=64 type=d alias=V103+0 align=32 words (r92.0)
//.declare V206 (214)  rf=r size=512 type=hf alias=V104+0 align=32 words (r222.0)
//.declare V207 (215)  rf=r size=512 type=hf alias=V105+0 align=32 words (r230.0)
//.declare V208 (216)  rf=r size=512 type=hf alias=V107+0 align=32 words (r238.0)
//.declare V209 (217)  rf=r size=512 type=hf alias=V108+0 align=32 words (r246.0)
//.declare  (218)  rf=r size=64 type=ud align=32 words (r255.0)
//.declare  (219)  rf=r size=2 type=uw align=1 words (r4.4)
//.declare r0 (220)  rf=r size=64 type=ud align=32 words (r0.0)
//.declare rtmp (221)  rf=r size=64 type=ud align=32 words (r255.0)
//.declare  (222)  rf=r size=64 type=ud align=32 words (r1.0)
//.declare  (223)  rf=r size=64 type=ud align=32 words (r2.0)
//.declare  (224)  rf=r size=4 type=ud align=2 words (r254.0)
//.declare  (225)  rf=r size=32 type=ud align=2 words (r3.0)

// .inputs
// +----------+----------+--------+----------+------------+
// | id       | type     |  bytes | at       | class      |
// +----------+----------+--------+----------+------------+
// | V32      | :d       |      4 | r2+24    | general    |
// | V47      | :uq      |      8 | r2+32    | general    |
// | V33      | :d       |      4 | r2+40    | general    |
// | V34      | :d       |      4 | r2+44    | general    |
// | V53      | :uq      |      8 | r2+48    | general    |
// | V35      | :d       |      4 | r2+56    | general    |
// | V36      | :d       |      4 | r2+60    | general    |
// | V96      | :uq      |      8 | r3       | general    |
// | V37      | :d       |      4 | r3+8     | general    |
// | V38      | :w x 3   |      6 | r1       | general    |
// | V39      | :d x 3   |     12 | r2       | general    |
// | V40      | :q       |      8 | r2+16    | general    |
// +----------+----------+--------+----------+------------+


// B000: Preds:{},  Succs:{B001}
per_thread_prolog:
(W)     mov (16|M0)              r255.0<1>:ud  0x0:ud                              {A@1}             // 
(W)     and (1|M0)               r255.2<1>:ud  r0.0<0;1,0>:ud    0xFFFFFFC0:ud                       // 
(W)     and (1|M0)               r255.0<1>:uw  r0.4<0;1,0>:uw    0xFF:uw                             // 
(W)     add (1|M0)               r255.2<1>:ud  r255.2<0;1,0>:ud  0x60:uw              {I@2}          // 
(W)     mad (1|M0)               r255.0<1>:ud  r255.2<0;0>:ud    r255.0<0;0>:uw    0x40:uw              {I@1} // 
(W)     load.ugm.d32x16t.a32.ca.ca (1|M0)  r1:1 bti[255][r255:1]   {A@1,$0} // ex_desc:0xFF000000; desc:0x6218D500 // 
        nop                                                                                          // 
        nop                                                                                          // 
// B001: Preds:{B000},  Succs:{B002}
// cross_thread_prolog:
(W)     and (1|M0)               r255.0<1>:ud  r0.0<0;1,0>:ud    0xFFFFFFC0:ud              {$0.src} // 
(W)     load.ugm.d32x16t.a32.ca.ca (1|M0)  r2:1 bti[255][r255:1]   {A@1,$1} // ex_desc:0xFF000000; desc:0x6218D500 // 
(W)     add (1|M0)               r254.0<1>:ud  r255.0<0;1,0>:ud  0x40:uw                             // 
(W)     load.ugm.d32x8t.a32.ca.ca (1|M0)  r3:1  bti[255][r254:1]   {I@1,$2} // ex_desc:0xFF000000; desc:0x6218C500 // 
// B002: Preds:{B001},  Succs:{B003, B005}
// _ZTSZZ16basic_brgemm_runjENKUlRN4sycl3_V17handlerEE_clES2_EUlNS0_7nd_itemILi3EEEE__BB_0:
        sync.nop                             null                             {Compacted,$1.dst}     // $4
        mul (2|M0)               r1.3<1>:d     r2.0<1;1,0>:d     r1.2<0;1,0>:uw   {$0.dst}           // $4
        mov (1|M0)               r1.2<1>:d     r0.6<0;1,0>:ud                                        // $1
        shl (1|M0)               r1.11<1>:d    r2.6<0;1,0>:d     1:w                                 // $11
        shl (1|M0)               r1.12<1>:d    r2.11<0;1,0>:d    1:w                                 // $14
        mov (1|M0)               r5.0<1>:q     r2.4<0;1,0>:q                    {Compacted}          // $10
        add (2|M0)               r1.5<1>:d     r1.3<1;1,0>:d     r1.0<1;1,0>:uw   {I@5}              // $5
        shl (1|M0)               r1.2<1>:d     r1.2<0;1,0>:d     8:w               {Compacted,I@5}   // $3
        add (1|M0)               r5.3<1>:d     r2.10<0;1,0>:d    -1:w                                // $13
        mov (1|M0)               r5.5<1>:d     0:w                                                   // $16
        add (1|M0)               r5.2<1>:d     r1.11<0;1,0>:d    -1:w               {I@7}            // $12
(W)     mul (2|M0)               acc0.0<1>:d   r1.6<0;1,0>:d     r2.0<2;1,0>:uw   {Compacted,I@5}    // $6
        add (1|M0)               r5.4<1>:d     r1.12<0;1,0>:d    -1:w                                // $15
        mov (1|M0)               r1.13<1>:d    r0.1<0;1,0>:ud                                        // $18
        macl (2|M0)              r4.0<1>:d     r1.6<0;1,0>:d     r2.0<1;1,0>:d                       // $7
        shl (1|M0)               r2.3<1>:d     r2.14<0;1,0>:d    1:w                                 // $24
        shl (1|M0)               r2.4<1>:d     r2.15<0;1,0>:d    1:w                                 // $27
        mov (1|M0)               r7.0<1>:q     r2.6<0;1,0>:q                    {Compacted}          // $23
        shl (1|M0)               r1.13<1>:d    r1.13<0;1,0>:d    8:w               {I@5}             // $19
        add (2|M0)               r1.8<1>:d     r4.0<1;1,0>:d     r1.0<1;1,0>:uw   {I@5}              // $7
        add (1|M0)               r7.3<1>:ud    r2.6<0;1,0>:ud    0xFFFFFFFF:ud                       // $26
        mov (1|M0)               r7.6<1>:d     0:w                                                   // $30
        add (1|M0)               r7.2<1>:d     r2.3<0;1,0>:d     -1:w               {I@7}            // $25
        add (1|M0)               r7.4<1>:d     r2.4<0;1,0>:d     -1:w               {Compacted,I@7}  // $28
        sync.nop                             null                             {Compacted,I@5}        // $8
        shr (1|M0)               r3.3<1>:ud    r1.8<0;1,0>:ud    0x2:uw              {$2.dst}        // $8
        and (1|M0)               r2.5<1>:d     r1.8<0;1,0>:d     3:w                                 // $21
        cmp (1|M0)    (lt)f0.1   null<1>:ud    r2.6<0;1,0>:ud    0x10:uw                             // $81
        mov (32|M0)              r126.0<1>:d   0:w                               {Compacted}         // $54
        shl (1|M0)               r2.7<1>:ud    r3.3<0;1,0>:ud    0x5:uw              {I@4}           // $9
        shl (1|M0)               r1.15<1>:d    r2.5<0;1,0>:d     6:w               {I@4}             // $22
        shl (1|M0)               r2.5<1>:d     r2.5<0;1,0>:d     3:w                                 // $31
        shl (1|M0)               r3.3<1>:ud    r3.3<0;1,0>:ud    0x1:uw                              // $38
        mov (32|M0)              r128.0<1>:d   0:w                               {Compacted}         // $55
        add (1|M0)               r5.6<1>:d     r1.2<0;1,0>:d     r2.7<0;1,0>:d    {I@5}              // $17
        or (1|M0)                r7.5<1>:d     r1.13<0;1,0>:d    r1.15<0;1,0>:d   {I@5}              // $29
        and (1|M0)               r3.3<1>:ud    r3.3<0;1,0>:ud    0x7FFFFFFC:ud              {I@4}    // $39
        and (1|M0)               r2.7<1>:d     r2.7<0;1,0>:d     32:w                                // $35
        mov (32|M0)              r130.0<1>:d   0:w                               {Compacted}         // $56
        mov (16|M0)              r9.0<1>:f     r5.0<1;1,0>:f                    {Compacted,I@5}      // $32
        add (1|M0)               r9.6<1>:d     r5.6<0;1,0>:d     r2.5<0;1,0>:d    {F@1}              // $33
        mov (1|M0)               r9.7<1>:d     1807:w                                                // $34
        mov (16|M0)              r10.0<1>:f    r7.0<1;1,0>:f                    {Compacted,I@6}      // $36
        add (1|M0)               r10.5<1>:d    r7.5<0;1,0>:d     r2.7<0;1,0>:d    {A@1}              // $37
        mov (1|M0)               r10.7<1>:d    799:w                                                 // $41
        load_block2d.ugm.d16.a64.ca.ca (1|M0)  null:0 [r9:1]       {A@3,$3} // ex_desc:0x0; desc:0x2080203 // $42
        add (1|M0)               r10.6<1>:ud   r10.6<0;1,0>:ud   r3.3<0;1,0>:ud                      // $40
        add (1|M0)               r9.5<1>:d     r9.5<0;1,0>:d     16:w               {$3.src}         // $44
        mov (32|M0)              r132.0<1>:d   0:w                               {Compacted}         // $57
        mov (32|M0)              r134.0<1>:d   0:w                               {Compacted}         // $58
        mov (32|M0)              r136.0<1>:d   0:w                               {Compacted}         // $59
        mov (32|M0)              r138.0<1>:d   0:w                               {Compacted}         // $60
        mov (32|M0)              r140.0<1>:d   0:w                               {Compacted}         // $61
        load_block2d.ugm.d16.a64.ca.ca (1|M0)  null:0 [r10:1]      {A@7,$4} // ex_desc:0x0; desc:0x2080203 // $43
        add (1|M0)               r10.6<1>:d    r10.6<0;1,0>:d    16:w               {$4.src}         // $45
        mov (32|M0)              r142.0<1>:d   0:w                               {Compacted}         // $62
        mov (32|M0)              r144.0<1>:d   0:w                               {Compacted}         // $63
        mov (32|M0)              r146.0<1>:d   0:w                               {Compacted}         // $64
        mov (32|M0)              r148.0<1>:d   0:w                               {Compacted}         // $65
        mov (32|M0)              r150.0<1>:d   0:w                               {Compacted}         // $66
        load_block2d.ugm.d16.a64.ca.ca (1|M0)  null:0 [r9:1]       {$5} // ex_desc:0x0; desc:0x2080203 // $46
        add (1|M0)               r9.5<1>:d     r9.5<0;1,0>:d     16:w               {$5.src}         // $48
        mov (32|M0)              r152.0<1>:d   0:w                               {Compacted}         // $67
        mov (32|M0)              r154.0<1>:d   0:w                               {Compacted}         // $68
        mov (32|M0)              r156.0<1>:d   0:w                               {Compacted}         // $69
        mov (32|M0)              r158.0<1>:d   0:w                               {Compacted}         // $70
        mov (32|M0)              r160.0<1>:d   0:w                               {Compacted}         // $71
        load_block2d.ugm.d16.a64.ca.ca (1|M0)  null:0 [r10:1]      {$6} // ex_desc:0x0; desc:0x2080203 // $47
        add (1|M0)               r10.6<1>:d    r10.6<0;1,0>:d    16:w               {$6.src}         // $49
        mov (32|M0)              r162.0<1>:d   0:w                               {Compacted}         // $72
        mov (32|M0)              r164.0<1>:d   0:w                               {Compacted}         // $73
        mov (32|M0)              r166.0<1>:d   0:w                               {Compacted}         // $74
        mov (32|M0)              r168.0<1>:d   0:w                               {Compacted}         // $75
        mov (32|M0)              r170.0<1>:d   0:w                               {Compacted}         // $76
        load_block2d.ugm.d16.a64.ca.ca (1|M0)  null:0 [r9:1]       {$7} // ex_desc:0x0; desc:0x2080203 // $50
        mov (32|M0)              r172.0<1>:d   0:w                               {Compacted}         // $77
        mov (32|M0)              r174.0<1>:d   0:w                               {Compacted}         // $78
        mov (32|M0)              r176.0<1>:d   0:w                               {Compacted}         // $79
        mov (32|M0)              r178.0<1>:d   0:w                               {Compacted}         // $80
        mov (32|M0)              r180.0<1>:d   0:w                               {Compacted}         // $82
        mov (32|M0)              r182.0<1>:d   0:w                               {Compacted}         // $83
        mov (32|M0)              r184.0<1>:d   0:w                               {Compacted}         // $84
        mov (32|M0)              r186.0<1>:d   0:w                               {Compacted}         // $85
        mov (32|M0)              r188.0<1>:d   0:w                               {Compacted}         // $86
        load_block2d.ugm.d16.a64.ca.ca (1|M0)  null:0 [r10:1]      {$8} // ex_desc:0x0; desc:0x2080203 // $51
        shr (1|M0)               r1.14<1>:ud   r2.6<0;1,0>:ud    0x4:uw                              // $20
        add (1|M0)               r9.5<1>:d     r9.5<0;1,0>:d     16:w               {$7.src}         // $52
        add (1|M0)               r10.6<1>:d    r10.6<0;1,0>:d    16:w               {$8.src}         // $53
(W&f0.1) jmpi                                BB_1                                                    // $87
// B003: Preds:{B002},  Succs:{B004}
_L_k0_0_:
        mov (16|M0)              r8.0<1>:f     r7.0<1;1,0>:f                    {Compacted}          // $90
        mov (16|M0)              r6.0<1>:f     r5.0<1;1,0>:f                    {Compacted}          // $92
        sel (1|M0)    (ge)f0.0   r1.14<1>:ud   r1.14<0;1,0>:ud   0x1:uw              {I@4}           // $88
        mov (1|M0)               r3.4<1>:d     0:w                               {Compacted}         // $89
        add (1|M0)               r8.5<1>:d     r7.5<0;1,0>:d     32:w               {F@2}            // $91
        add (1|M0)               r6.6<1>:d     r5.6<0;1,0>:d     16:w               {F@1}            // $93
// B004: Preds:{B004, B003},  Succs:{B005, B004}
BB_2:
        mov (1|M0)               r7.7<1>:f     0.0:f                                                 //  (0x00010f0f:f); $95
        mov (1|M0)               r8.7<1>:f     0.0:f                                                 //  (0x00010f0f:f); $96
        mov (1|M0)               r5.7<1>:d     3855:w                                                // $99
        mov (1|M0)               r6.7<1>:d     3855:w                                                // $101
        load_block2d.ugm.d16v.a64.ca.ca (1|M0)  r11:16 [r7:1]       {F@2,$10} // ex_desc:0x0; desc:0x3080283 // $97
        load_block2d.ugm.d16v.a64.ca.ca (1|M0)  r27:16 [r8:1]       {A@1,$11} // ex_desc:0x0; desc:0x3080283 // $98
        sync.nop                             null                             {Compacted,$9.src}     // $103
        mov (32|M0)              r59.0<1>:hf   r11.0<1;1,0>:hf                  {$10.dst}            // $103
        mov (32|M0)              r60.0<1>:hf   r12.0<1;1,0>:hf                                       // $104
        mov (32|M0)              r61.0<1>:hf   r13.0<1;1,0>:hf                                       // $105
        mov (32|M0)              r62.0<1>:hf   r14.0<1;1,0>:hf                                       // $106
        mov (32|M0)              r63.0<1>:hf   r15.0<1;1,0>:hf                                       // $107
        mov (32|M0)              r64.0<1>:hf   r16.0<1;1,0>:hf                                       // $108
        load_block2d.ugm.d16.a64.ca.ca (1|M0)  r43:8 [r5:1]        {I@2,$12} // ex_desc:0x0; desc:0x2880203 // $100
        mov (32|M0)              r65.0<1>:hf   r17.0<1;1,0>:hf                                       // $109
        mov (32|M0)              r66.0<1>:hf   r18.0<1;1,0>:hf                                       // $110
        mov (32|M0)              r67.0<1>:hf   r19.0<1;1,0>:hf                                       // $111
        mov (32|M0)              r68.0<1>:hf   r20.0<1;1,0>:hf                                       // $112
        mov (32|M0)              r69.0<1>:hf   r21.0<1;1,0>:hf                                       // $113
        mov (32|M0)              r70.0<1>:hf   r22.0<1;1,0>:hf                                       // $114
        load_block2d.ugm.d16.a64.ca.ca (1|M0)  r51:8 [r6:1]        {I@1,$13} // ex_desc:0x0; desc:0x2880203 // $102
        mov (32|M0)              r71.0<1>:hf   r23.0<1;1,0>:hf                                       // $115
        mov (32|M0)              r72.0<1>:hf   r24.0<1;1,0>:hf                                       // $116
        mov (32|M0)              r73.0<1>:hf   r25.0<1;1,0>:hf                                       // $117
        mov (32|M0)              r74.0<1>:hf   r26.0<1;1,0>:hf                                       // $118
        load_block2d.ugm.d16.a64.ca.ca (1|M0)  null:0 [r9:1]       {$14} // ex_desc:0x0; desc:0x2080203 // $119
        load_block2d.ugm.d16.a64.ca.ca (1|M0)  null:0 [r10:1]      {$15} // ex_desc:0x0; desc:0x2080203 // $120
        mov (32|M0)              r95.0<1>:hf   r43.0<1;1,0>:hf                  {$12.dst}            // $123
        mov (32|M0)              r96.0<1>:hf   r44.0<1;1,0>:hf                                       // $124
        mov (32|M0)              r97.0<1>:hf   r45.0<1;1,0>:hf                                       // $125
        mov (32|M0)              r98.0<1>:hf   r46.0<1;1,0>:hf                                       // $126
        mov (32|M0)              r103.0<1>:hf  r47.0<1;1,0>:hf                                       // $127
        mov (32|M0)              r104.0<1>:hf  r48.0<1;1,0>:hf                                       // $128
        mov (32|M0)              r105.0<1>:hf  r49.0<1;1,0>:hf                                       // $129
        mov (32|M0)              r106.0<1>:hf  r50.0<1;1,0>:hf                                       // $130
        mov (32|M0)              r111.0<1>:hf  r51.0<1;1,0>:hf                  {$13.dst}            // $131
        mov (32|M0)              r112.0<1>:hf  r52.0<1;1,0>:hf                                       // $132
        mov (32|M0)              r113.0<1>:hf  r53.0<1;1,0>:hf                                       // $133
        mov (32|M0)              r114.0<1>:hf  r54.0<1;1,0>:hf                                       // $134
        mov (32|M0)              r119.0<1>:hf  r55.0<1;1,0>:hf                                       // $135
        mov (32|M0)              r120.0<1>:hf  r56.0<1;1,0>:hf                                       // $136
        mov (32|M0)              r121.0<1>:hf  r57.0<1;1,0>:hf                                       // $137
        mov (32|M0)              r122.0<1>:w   r58.0<1;1,0>:w                                        // $138
        mov (32|M0)              r75.0<1>:w    r27.0<1;1,0>:w                   {$11.dst}            // $141
        mov (32|M0)              r76.0<1>:w    r28.0<1;1,0>:w                                        // $142
        mov (32|M0)              r77.0<1>:w    r29.0<1;1,0>:w                                        // $143
        mov (32|M0)              r78.0<1>:w    r30.0<1;1,0>:w                                        // $144
        mov (32|M0)              r79.0<1>:w    r31.0<1;1,0>:w                                        // $145
        mov (32|M0)              r80.0<1>:w    r32.0<1;1,0>:w                                        // $146
        mov (32|M0)              r81.0<1>:w    r33.0<1;1,0>:w                                        // $147
        mov (32|M0)              r82.0<1>:w    r34.0<1;1,0>:w                                        // $148
        mov (32|M0)              r83.0<1>:w    r35.0<1;1,0>:w                                        // $149
        mov (32|M0)              r84.0<1>:w    r36.0<1;1,0>:w                                        // $150
        mov (32|M0)              r85.0<1>:w    r37.0<1;1,0>:w                                        // $151
        mov (32|M0)              r86.0<1>:w    r38.0<1;1,0>:w                                        // $152
        mov (32|M0)              r87.0<1>:w    r39.0<1;1,0>:w                                        // $153
        mov (32|M0)              r88.0<1>:w    r40.0<1;1,0>:w                                        // $154
        mov (32|M0)              r89.0<1>:w    r41.0<1;1,0>:w                                        // $155
        mov (32|M0)              r90.0<1>:w    r42.0<1;1,0>:w                                        // $156
        add (1|M0)               r5.5<1>:d     r5.5<0;1,0>:d     16:w                                // $122
        add (1|M0)               r7.6<1>:d     r7.6<0;1,0>:d     16:w                                // $139
        add (1|M0)               r8.6<1>:d     r8.6<0;1,0>:d     16:w                                // $140
        dpas.8x8 (16|M0)         r126:hf       r126:hf           r59:hf            r95.0:hf         {Atomic,Compacted,A@1} // $159
        dpas.8x8 (16|M0)         r130:hf       r130:hf           r59:hf            r103.0:hf        {Atomic,Compacted} // $160
        dpas.8x8 (16|M0)         r158:hf       r158:hf           r59:hf            r111.0:hf        {Atomic,Compacted} // $161
        dpas.8x8 (16|M0)         r162:hf       r162:hf           r59:hf            r119.0:hf        {Atomic,Compacted} // $162
        dpas.8x8 (16|M0)         r134:hf       r134:hf           r67:hf            r95.0:hf         {Atomic,Compacted} // $163
        dpas.8x8 (16|M0)         r138:hf       r138:hf           r67:hf            r103.0:hf        {Atomic,Compacted} // $164
        dpas.8x8 (16|M0)         r166:hf       r166:hf           r67:hf            r111.0:hf        {Atomic,Compacted} // $165
        dpas.8x8 (16|M0)         r170:hf       r170:hf           r67:hf            r119.0:hf        {Atomic,Compacted} // $166
        dpas.8x8 (16|M0)         r142:hf       r142:hf           r75:hf            r95.0:hf         {Atomic,Compacted} // $167
        dpas.8x8 (16|M0)         r146:hf       r146:hf           r75:hf            r103.0:hf        {Atomic,Compacted} // $168
        dpas.8x8 (16|M0)         r174:hf       r174:hf           r75:hf            r111.0:hf        {Atomic,Compacted} // $169
        dpas.8x8 (16|M0)         r178:hf       r178:hf           r75:hf            r119.0:hf        {Atomic,Compacted} // $170
        dpas.8x8 (16|M0)         r150:hf       r150:hf           r83:hf            r95.0:hf         {Atomic,Compacted} // $171
        dpas.8x8 (16|M0)         r154:hf       r154:hf           r83:hf            r103.0:hf        {Atomic,Compacted} // $172
        dpas.8x8 (16|M0)         r182:hf       r182:hf           r83:hf            r111.0:hf        {Atomic,Compacted} // $173
        dpas.8x8 (16|M0)         r186:hf       r186:hf           r83:hf            r119.0:hf        {$9} // $174
        add (1|M0)               r6.5<1>:d     r6.5<0;1,0>:d     16:w                                // $175
        add (1|M0)               r9.5<1>:d     r9.5<0;1,0>:d     16:w               {$14.src}        // $176
        add (1|M0)               r10.6<1>:d    r10.6<0;1,0>:d    16:w               {$15.src}        // $177
        add (1|M0)               r3.4<1>:d     r3.4<0;1,0>:d     1:w               {Compacted}       // $179
        cmp (1|M0)    (eq)f0.0   null<1>:d     r3.4<0;1,0>:d     r1.14<0;1,0>:d   {I@1}              // $180
(W&~f0.0) jmpi                               BB_2                                                    // $181
// B005: Preds:{B004, B002},  Succs:{}
BB_1:
        shl (1|M0)               r3.6<1>:d     r1.8<0;1,0>:d     3:w                                 // $185
        shl (1|M0)               r3.5<1>:d     r1.8<0;1,0>:d     6:w                                 // $184
        shl (1|M0)               r3.7<1>:d     r3.2<0;1,0>:d     1:w                                 // $190
(W)     mov (1|M0)               r4.4<1>:hf    0xC0:hf                                               // $192
        mov (1|M0)               r94.0<1>:q    r3.0<0;1,0>:q                    {Compacted}          // $187
        and (1|M0)               r3.6<1>:d     r3.6<0;1,0>:d     -32:w               {I@4}           // $186
        add (1|M0)               r94.2<1>:d    r2.3<0;1,0>:d     -1:w                                // $188
        add (1|M0)               r94.3<1>:d    r2.10<0;1,0>:d    -1:w                                // $189
        add (1|M0)               r94.4<1>:d    r3.7<0;1,0>:d     -1:w               {I@5}            // $191
        bfn.(s0&s1|s2) (1|M0)    r94.5<1>:ud   r3.5<0;0>:ud      r4.4<0;0>:uw      r1.13<0>:ud      {F@1} // $192
        add (1|M0)               r94.6<1>:d    r1.2<0;1,0>:d     r3.6<0;1,0>:d    {I@5}              // $193
        mov (16|M0)              r190.0<1>:hf  r126.0<1;1,0>:hf                 {$9.dst}             // $202
        mov (16|M0)              r191.0<1>:uw  r126.16<1;1,0>:uw                                     // $203
        mov (16|M0)              r192.0<1>:hf  r127.0<1;1,0>:hf                                      // $204
        mov (16|M0)              r91.0<1>:f    r94.0<1;1,0>:f                   {Compacted,I@2}      // $194
        mov (16|M0)              r193.0<1>:uw  r127.16<1;1,0>:uw                                     // $205
        mov (16|M0)              r194.0<1>:hf  r128.0<1;1,0>:hf                                      // $206
        mov (16|M0)              r195.0<1>:uw  r128.16<1;1,0>:uw                                     // $207
        mov (16|M0)              r196.0<1>:hf  r129.0<1;1,0>:hf                                      // $208
        mov (16|M0)              r197.0<1>:uw  r129.16<1;1,0>:uw                                     // $209
        mov (16|M0)              r190.16<1>:uw  r134.0<1;1,0>:uw                                     // $218
        mov (16|M0)              r191.16<1>:hf  r134.16<1;1,0>:hf                                    // $219
        mov (16|M0)              r192.16<1>:uw  r135.0<1;1,0>:uw                                     // $220
        mov (16|M0)              r193.16<1>:hf  r135.16<1;1,0>:hf                                    // $221
        mov (16|M0)              r194.16<1>:uw  r136.0<1;1,0>:uw                                     // $222
        mov (16|M0)              r195.16<1>:hf  r136.16<1;1,0>:hf                                    // $223
        mov (16|M0)              r196.16<1>:uw  r137.0<1;1,0>:uw                                     // $224
        mov (16|M0)              r197.16<1>:hf  r137.16<1;1,0>:hf                                    // $225
        mov (1|M0)               r91.7<1>:d    1823:w                               {F@7}            // $195
        mov (16|M0)              r198.0<1>:hf  r130.0<1;1,0>:hf                                      // $210
        mov (16|M0)              r199.0<1>:uw  r130.16<1;1,0>:uw                                     // $211
        mov (16|M0)              r200.0<1>:hf  r131.0<1;1,0>:hf                                      // $212
        store_block2d.ugm.d16.a64.wb.wb (1|M0)  [r91:1] r190:8     {A@2,$16} // ex_desc:0x0; desc:0x20E0207 // $234
        mov (16|M0)              r201.0<1>:uw  r131.16<1;1,0>:uw                                     // $213
        mov (16|M0)              r202.0<1>:hf  r132.0<1;1,0>:hf                                      // $214
        mov (16|M0)              r203.0<1>:uw  r132.16<1;1,0>:uw                                     // $215
        mov (16|M0)              r204.0<1>:hf  r133.0<1;1,0>:hf                                      // $216
        mov (16|M0)              r205.0<1>:uw  r133.16<1;1,0>:uw                                     // $217
        mov (16|M0)              r198.16<1>:uw  r138.0<1;1,0>:uw                                     // $226
        mov (16|M0)              r199.16<1>:hf  r138.16<1;1,0>:hf                                    // $227
        mov (16|M0)              r200.16<1>:uw  r139.0<1;1,0>:uw                                     // $228
        mov (16|M0)              r201.16<1>:hf  r139.16<1;1,0>:hf                                    // $229
        mov (16|M0)              r202.16<1>:uw  r140.0<1;1,0>:uw                                     // $230
        mov (16|M0)              r203.16<1>:hf  r140.16<1;1,0>:hf                                    // $231
        mov (16|M0)              r204.16<1>:uw  r141.0<1;1,0>:uw                                     // $232
        mov (16|M0)              r205.16<1>:hf  r141.16<1;1,0>:hf                                    // $233
        add (1|M0)               r91.6<1>:d    r91.6<0;1,0>:d    8:w               {$16.src}         // $235
        mov (16|M0)              r93.0<1>:f    r94.0<1;1,0>:f                   {Compacted}          // $196
        add (1|M0)               r94.5<1>:d    r94.5<0;1,0>:d    32:w               {F@1}            // $198
        mov (16|M0)              r206.0<1>:hf  r142.0<1;1,0>:hf                                      // $237
        mov (16|M0)              r207.0<1>:uw  r142.16<1;1,0>:uw                                     // $238
        mov (16|M0)              r208.0<1>:hf  r143.0<1;1,0>:hf                                      // $239
        mov (16|M0)              r92.0<1>:f    r94.0<1;1,0>:f                   {Compacted,I@2}      // $199
        mov (16|M0)              r209.0<1>:uw  r143.16<1;1,0>:uw                                     // $240
        mov (16|M0)              r210.0<1>:hf  r144.0<1;1,0>:hf                                      // $241
        mov (16|M0)              r211.0<1>:uw  r144.16<1;1,0>:uw                                     // $242
        mov (16|M0)              r212.0<1>:hf  r145.0<1;1,0>:hf                                      // $243
        mov (16|M0)              r213.0<1>:uw  r145.16<1;1,0>:uw                                     // $244
        mov (16|M0)              r206.16<1>:uw  r150.0<1;1,0>:uw                                     // $253
        mov (16|M0)              r207.16<1>:hf  r150.16<1;1,0>:hf                                    // $254
        mov (16|M0)              r208.16<1>:uw  r151.0<1;1,0>:uw                                     // $255
        mov (16|M0)              r209.16<1>:hf  r151.16<1;1,0>:hf                                    // $256
        mov (16|M0)              r210.16<1>:uw  r152.0<1;1,0>:uw                                     // $257
        mov (16|M0)              r211.16<1>:hf  r152.16<1;1,0>:hf                                    // $258
        mov (16|M0)              r212.16<1>:uw  r153.0<1;1,0>:uw                                     // $259
        mov (16|M0)              r213.16<1>:hf  r153.16<1;1,0>:hf                                    // $260
        mov (1|M0)               r92.7<1>:d    1823:w                               {F@7}            // $200
        mov (16|M0)              r214.0<1>:hf  r146.0<1;1,0>:hf                                      // $245
        mov (16|M0)              r215.0<1>:uw  r146.16<1;1,0>:uw                                     // $246
        mov (16|M0)              r216.0<1>:hf  r147.0<1;1,0>:hf                                      // $247
        mov (16|M0)              r217.0<1>:uw  r147.16<1;1,0>:uw                                     // $248
        mov (16|M0)              r218.0<1>:hf  r148.0<1;1,0>:hf                                      // $249
        mov (16|M0)              r219.0<1>:uw  r148.16<1;1,0>:uw                                     // $250
        mov (16|M0)              r220.0<1>:hf  r149.0<1;1,0>:hf                                      // $251
        mov (16|M0)              r221.0<1>:uw  r149.16<1;1,0>:uw                                     // $252
        mov (16|M0)              r214.16<1>:uw  r154.0<1;1,0>:uw                                     // $261
        mov (16|M0)              r215.16<1>:hf  r154.16<1;1,0>:hf                                    // $262
        mov (16|M0)              r216.16<1>:uw  r155.0<1;1,0>:uw                                     // $263
        mov (16|M0)              r217.16<1>:hf  r155.16<1;1,0>:hf                                    // $264
        mov (16|M0)              r218.16<1>:uw  r156.0<1;1,0>:uw                                     // $265
        mov (16|M0)              r219.16<1>:hf  r156.16<1;1,0>:hf                                    // $266
        mov (16|M0)              r220.16<1>:uw  r157.0<1;1,0>:uw                                     // $267
        mov (16|M0)              r221.16<1>:hf  r157.16<1;1,0>:hf                                    // $268
        mov (16|M0)              r222.0<1>:hf  r158.0<1;1,0>:hf                                      // $274
        mov (16|M0)              r223.0<1>:uw  r158.16<1;1,0>:uw                                     // $275
        mov (16|M0)              r224.0<1>:hf  r159.0<1;1,0>:hf                                      // $276
        mov (16|M0)              r225.0<1>:uw  r159.16<1;1,0>:uw                                     // $277
        mov (16|M0)              r226.0<1>:hf  r160.0<1;1,0>:hf                                      // $278
        mov (16|M0)              r227.0<1>:uw  r160.16<1;1,0>:uw                                     // $279
        mov (16|M0)              r228.0<1>:hf  r161.0<1;1,0>:hf                                      // $280
        mov (16|M0)              r229.0<1>:uw  r161.16<1;1,0>:uw                                     // $281
        store_block2d.ugm.d16.a64.wb.wb (1|M0)  [r91:1] r198:8     {$17} // ex_desc:0x0; desc:0x20E0207 // $236
        mov (16|M0)              r222.16<1>:uw  r166.0<1;1,0>:uw                                     // $290
        mov (16|M0)              r223.16<1>:hf  r166.16<1;1,0>:hf                                    // $291
        mov (16|M0)              r224.16<1>:uw  r167.0<1;1,0>:uw                                     // $292
        mov (16|M0)              r225.16<1>:hf  r167.16<1;1,0>:hf                                    // $293
        mov (16|M0)              r226.16<1>:uw  r168.0<1;1,0>:uw                                     // $294
        mov (16|M0)              r227.16<1>:hf  r168.16<1;1,0>:hf                                    // $295
        mov (16|M0)              r228.16<1>:uw  r169.0<1;1,0>:uw                                     // $296
        mov (16|M0)              r229.16<1>:hf  r169.16<1;1,0>:hf                                    // $297
        add (1|M0)               r93.6<1>:d    r94.6<0;1,0>:d    16:w                                // $197
        mov (1|M0)               r93.7<1>:d    1823:w                                                // $272
        mov (16|M0)              r230.0<1>:hf  r162.0<1;1,0>:hf                                      // $282
        mov (16|M0)              r231.0<1>:uw  r162.16<1;1,0>:uw                                     // $283
        mov (16|M0)              r232.0<1>:hf  r163.0<1;1,0>:hf                                      // $284
        mov (16|M0)              r233.0<1>:uw  r163.16<1;1,0>:uw                                     // $285
        mov (16|M0)              r234.0<1>:hf  r164.0<1;1,0>:hf                                      // $286
        mov (16|M0)              r235.0<1>:uw  r164.16<1;1,0>:uw                                     // $287
        mov (16|M0)              r236.0<1>:hf  r165.0<1;1,0>:hf                                      // $288
        mov (16|M0)              r237.0<1>:uw  r165.16<1;1,0>:uw                                     // $289
        mov (16|M0)              r230.16<1>:uw  r170.0<1;1,0>:uw                                     // $298
        mov (16|M0)              r231.16<1>:hf  r170.16<1;1,0>:hf                                    // $299
        mov (16|M0)              r232.16<1>:uw  r171.0<1;1,0>:uw                                     // $300
        mov (16|M0)              r233.16<1>:hf  r171.16<1;1,0>:hf                                    // $301
        mov (16|M0)              r234.16<1>:uw  r172.0<1;1,0>:uw                                     // $302
        mov (16|M0)              r235.16<1>:hf  r172.16<1;1,0>:hf                                    // $303
        mov (16|M0)              r236.16<1>:uw  r173.0<1;1,0>:uw                                     // $304
        mov (16|M0)              r237.16<1>:hf  r173.16<1;1,0>:hf                                    // $305
        mov (16|M0)              r238.0<1>:hf  r174.0<1;1,0>:hf                                      // $309
        mov (16|M0)              r239.0<1>:uw  r174.16<1;1,0>:uw                                     // $310
        mov (16|M0)              r240.0<1>:hf  r175.0<1;1,0>:hf                                      // $311
        mov (16|M0)              r241.0<1>:uw  r175.16<1;1,0>:uw                                     // $312
        mov (16|M0)              r242.0<1>:hf  r176.0<1;1,0>:hf                                      // $313
        mov (16|M0)              r243.0<1>:uw  r176.16<1;1,0>:uw                                     // $314
        mov (16|M0)              r244.0<1>:hf  r177.0<1;1,0>:hf                                      // $315
        mov (16|M0)              r245.0<1>:uw  r177.16<1;1,0>:uw                                     // $316
        mov (16|M0)              r238.16<1>:uw  r182.0<1;1,0>:uw                                     // $325
        mov (16|M0)              r239.16<1>:hf  r182.16<1;1,0>:hf                                    // $326
        mov (16|M0)              r240.16<1>:uw  r183.0<1;1,0>:uw                                     // $327
        mov (16|M0)              r241.16<1>:hf  r183.16<1;1,0>:hf                                    // $328
        mov (16|M0)              r242.16<1>:uw  r184.0<1;1,0>:uw                                     // $329
        mov (16|M0)              r243.16<1>:hf  r184.16<1;1,0>:hf                                    // $330
        mov (16|M0)              r244.16<1>:uw  r185.0<1;1,0>:uw                                     // $331
        mov (16|M0)              r245.16<1>:hf  r185.16<1;1,0>:hf                                    // $332
        mov (1|M0)               r94.7<1>:d    1823:w                                                // $273
        add (1|M0)               r94.6<1>:d    r94.6<0;1,0>:d    16:w                                // $201
        mov (16|M0)              r246.0<1>:hf  r178.0<1;1,0>:hf                                      // $317
        mov (16|M0)              r247.0<1>:uw  r178.16<1;1,0>:uw                                     // $318
        mov (16|M0)              r248.0<1>:hf  r179.0<1;1,0>:hf                                      // $319
        mov (16|M0)              r249.0<1>:uw  r179.16<1;1,0>:uw                                     // $320
        mov (16|M0)              r250.0<1>:hf  r180.0<1;1,0>:hf                                      // $321
        mov (16|M0)              r251.0<1>:uw  r180.16<1;1,0>:uw                                     // $322
        mov (16|M0)              r252.0<1>:hf  r181.0<1;1,0>:hf                                      // $323
        mov (16|M0)              r253.0<1>:uw  r181.16<1;1,0>:uw                                     // $324
        mov (16|M0)              r246.16<1>:uw  r186.0<1;1,0>:uw                                     // $333
        mov (16|M0)              r247.16<1>:hf  r186.16<1;1,0>:hf                                    // $334
        mov (16|M0)              r248.16<1>:uw  r187.0<1;1,0>:uw                                     // $335
        mov (16|M0)              r249.16<1>:hf  r187.16<1;1,0>:hf                                    // $336
        mov (16|M0)              r250.16<1>:uw  r188.0<1;1,0>:uw                                     // $337
        mov (16|M0)              r251.16<1>:hf  r188.16<1;1,0>:hf                                    // $338
        store_block2d.ugm.d16.a64.wb.wb (1|M0)  [r92:1] r206:8     {$18} // ex_desc:0x0; desc:0x20E0207 // $269
        add (1|M0)               r92.6<1>:d    r92.6<0;1,0>:d    8:w               {$18.src}         // $270
        mov (16|M0)              r252.16<1>:uw  r189.0<1;1,0>:uw                                     // $339
        mov (16|M0)              r253.16<1>:hf  r189.16<1;1,0>:hf                                    // $340
(W)     mov (16|M0)              r255.0<1>:f   r0.0<1;1,0>:f                    {Compacted}          // $344
        store_block2d.ugm.d16.a64.wb.wb (1|M0)  [r92:1] r214:8     {I@2,$19} // ex_desc:0x0; desc:0x20E0207 // $271
        store_block2d.ugm.d16.a64.wb.wb (1|M0)  [r93:1] r222:8     {$20} // ex_desc:0x0; desc:0x20E0207 // $306
        add (1|M0)               r93.6<1>:d    r93.6<0;1,0>:d    8:w               {$20.src}         // $307
        store_block2d.ugm.d16.a64.wb.wb (1|M0)  [r93:1] r230:8     {I@1,$21} // ex_desc:0x0; desc:0x20E0207 // $308
        store_block2d.ugm.d16.a64.wb.wb (1|M0)  [r94:1] r238:8     {A@7,$22} // ex_desc:0x0; desc:0x20E0207 // $341
        add (1|M0)               r94.6<1>:d    r94.6<0;1,0>:d    8:w               {$22.src}         // $342
        store_block2d.ugm.d16.a64.wb.wb (1|M0)  [r94:1] r246:8     {A@1,$23} // ex_desc:0x0; desc:0x20E0207 // $343
(W)     send.gtwy (1|M0)         null     r255    null:0  0x0            0x02000010           {EOT,F@1,$24} // wr:1+0, rd:0; end of thread // $344
L5136:
        nop                                                                                          // $344


//.BankConflicts: 0
//.ByteRMWs: 0
//


//.numALUInst: 311
//.accSubDef: 0
//.accSubUse: 0
//.accSubCandidateDef: 0
//.accSubCandidateUse: 0
//
//
//.singlePipeAtOneDistNum: 10
//.allAtOneDistNum: 7
//.syncInstCount: 3
//.tokenReuseCount: 0
//.AfterWriteTokenDepCount: 8
//.AfterReadTokenDepCount: 14
