-- MySQL dump 10.13  Distrib 5.7.28, for Linux (x86_64)
--
-- Host: localhost    Database: Message
-- ------------------------------------------------------
-- Server version	5.7.28-0ubuntu0.16.04.2

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `message`
--

DROP TABLE IF EXISTS `message`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `message` (
  `mess_id` bigint(10) unsigned NOT NULL AUTO_INCREMENT,
  `mess_itemid` bigint(10) NOT NULL,
  `mess_time` varchar(50) NOT NULL,
  `mess_content` varchar(255) NOT NULL,
  `mess_username` varchar(255) NOT NULL,
  PRIMARY KEY (`mess_id`),
  UNIQUE KEY `mess_id_UNIQUE` (`mess_id`)
) ENGINE=InnoDB AUTO_INCREMENT=15 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `message`
--

LOCK TABLES `message` WRITE;
/*!40000 ALTER TABLE `message` DISABLE KEYS */;
INSERT INTO `message` VALUES (1,9,'2019-12-13 17:58:44','hi, man',''),(2,9,'2019-12-13 17:58:51','how are you?',''),(3,9,'2019-12-13 17:58:59','Good Try?',''),(4,9,'2019-12-13 17:59:05','你好',''),(5,9,'2019-12-13 17:59:34','很好',''),(6,9,'2019-12-13 17:59:40','不错',''),(7,9,'2019-12-13 18:04:45','how do you do ?','who'),(8,9,'2019-12-13 18:05:10','well','who'),(9,9,'2019-12-13 18:11:22','你说啥嘞','who'),(10,9,'2019-12-13 18:11:57','评价评价评价评价评价评价评价评价评价评价评价评价评价评价评价评价评价评价评价评价评价评价评价评价评价评价评价评价评价评价','who'),(11,9,'2019-12-13 18:12:24','辣是真的牛皮','who'),(12,9,'2019-12-13 18:12:35','大大是的','who'),(13,9,'2019-12-13 18:12:46','出租车在vczcz','who'),(14,9,'2019-12-13 18:12:53','vvvv','who');
/*!40000 ALTER TABLE `message` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2019-12-13 18:15:48
